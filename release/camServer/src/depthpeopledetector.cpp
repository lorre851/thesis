//
// Created by lorre851 on 31.03.17.
//

#include "depthpeopledetector.h"



/*
 *
 *      PRIVATE
 *
 */


void depthpeopledetector::aggergatekeypoints(std::vector<cv::KeyPoint> &points) {
    unsigned long multiplier = 1.5;
    double min_size = 35.0;
    double max_size = 50.0;
    bool has_aggregated = false;
    if(points.size() > 1) {
        //make keypoint size a bit bigger
        for(int i = 0; i < points.size(); i++) {
            points[i] = cv::KeyPoint(points[i].pt, points[i].size * multiplier);
        }
        //aggregate overlapping keypoints
        for (int i = 0; i < points.size(); i++) {
            for (int j = i + 1; j < points.size(); j++) {
                if(cv::KeyPoint::overlap(points[i], points[j]) > 0) {
                    points[i] = cv::KeyPoint(
                            cv::Point2f((points[i].pt.x + points[j].pt.x) / 2.0, (points[i].pt.y + points[j].pt.y) / 2.0),
                            points[i].size + points[j].size
                    );
                    points[j] = cv::KeyPoint(cv::Point2f(0, 0), 0.0);
                    has_aggregated = true;
                }
            }
        }


        //make keypoints a little bit smaller
        for(int i = 0; i < points.size(); i++) {

            points[i] = cv::KeyPoint(points[i].pt, points[i].size / multiplier);
            if(points[i].size < min_size) points[i] = cv::KeyPoint(cv::Point2f(0, 0), 0.0);
            else points[i] = cv::KeyPoint(points[i].pt, max_size);

        }

        //if(has_aggregated) aggergatekeypoints(points);


    }
}

/*
Bedjenkt, stackoverflow

def has_overlap(Ax1, Ax2, Ay1, Ay2, Bx1, Bx2, By1, By2):
	if(Ax1 < Bx2 and Ax2 > Bx1 and Ay1 < By2 and Ay2 > By1):
		return True
	else:
		return False
*/

bool depthpeopledetector::eliminate_overlap(struct person &p1, struct person &p2) {
    float Ax1 = p1.last_keypoint.pt.x - TRACK_REGION;
    float Ax2 = p1.last_keypoint.pt.x + TRACK_REGION;
    float Ay1 = p1.last_keypoint.pt.y - TRACK_REGION;
    float Ay2 = p1.last_keypoint.pt.y + TRACK_REGION;
    float Bx1 = p2.last_keypoint.pt.x - TRACK_REGION;
    float Bx2 = p2.last_keypoint.pt.x + TRACK_REGION;
    float By1 = p2.last_keypoint.pt.y - TRACK_REGION;
    float By2 = p2.last_keypoint.pt.y + TRACK_REGION;

    //condition for overlap
    if(Ax1 < Bx2 && Ax2 > Bx1 && Ay1 < By2 && Ay2 > By1) {

        //TODO: calculate overlap in rectangular regions and eliminate based on THAT threshold

        float intersection = std::max((float)(0.0), std::min(Ax2, Bx2 - std::max(Ax1, Bx1)) * std::max((float)(0.0), std::min(Ay2, By2) - std::max(Ay1, By1)));
        float area_a = abs(Ax2-Ax1) * abs(Ay2-Ay1);
        float area_b = abs(Ax2-Ax1) * abs(Ay2-Ay1);
        float ratio = intersection / (area_a + area_b - intersection);

        if(ratio > 0.13) {
            if (p1.tracked_for > p2.tracked_for) p2.misses = MISS_THRESH + 1;
            else p1.misses = MISS_THRESH + 1;
            if(dev) std::cout << "Eliminated an overlap, ratio was " << ratio << std::endl;
        }
        else {
            if(dev) std::cout << "Elimination ignored - ratio not big enough (was " << ratio << ")" << std::endl;
        }

    }
}

void depthpeopledetector::estimate_next(struct person &p) {
    if(p.tracked_for > 2) {
        p.estimated = (
                              (p.hist_2 - p.hist_1) +
                              (p.hist_3 - p.hist_2)
                      ) / 2.0;
    }
    else if(p.tracked_for > 1) {
        p.estimated = p.hist_2 - p.hist_1;
    }
}

void depthpeopledetector::push_point_history(struct person &p) {
    if(p.tracked_for > 0) {
        if (p.tracked_for == 1) p.hist_1 = p.last_keypoint.pt;
        else if (p.tracked_for == 2) p.hist_2 = p.last_keypoint.pt;
        else if (p.tracked_for == 3) p.estimated = p.hist_3 = p.last_keypoint.pt;
        else {
            p.hist_1 = p.hist_2;
            p.hist_2 = p.hist_3;
            p.hist_3 = p.last_keypoint.pt;
        }


        if(p.last_keypoint.pt.x > p.max.x || p.max.x == -1) p.max.x = p.last_keypoint.pt.x;
        if(p.last_keypoint.pt.y > p.max.y || p.max.y == -1) p.max.y = p.last_keypoint.pt.y;
        if(p.last_keypoint.pt.x < p.min.x || p.min.x == -1) p.min.x = p.last_keypoint.pt.x;
        if(p.last_keypoint.pt.y < p.min.y || p.min.y == -1) p.min.y = p.last_keypoint.pt.y;
    }
}


/*
 *
 *          PUBLIC
 *
 */


depthpeopledetector::depthpeopledetector() {
    rng = cv::RNG((uint64)seconds);
}



void depthpeopledetector::detect(cv::Mat &img) {

    cv::Mat median, thresh, roi, res, border, im_with_keypoints, keypoints_original;
    cv::SimpleBlobDetector::Params params;


    //Set blobdetector parameters

    // Filter by Area.
    params.filterByArea = true;
    params.minArea = 60;

    // Filter by Circularity
    params.filterByCircularity = false;
    params.minCircularity = 0.1;

    // Filter by Convexity
    params.filterByConvexity = false;
    params.minConvexity = 0.87;

    // Filter by Inertia
    params.filterByInertia = false;
    params.minInertiaRatio = 0.01;





    //cut noise from image (for development only - you don't need to do this with a dedicated stereo camera)
    img_roi = cv::Rect(90, 30, img.size().width - 260, img.size().height - 40);
    roi = img(img_roi);

    //add some blurring, continue with setting threshold (higher than = 0, lower than = 1)
    medianBlur(roi, median, 5);
    threshold(median, thresh, 60, 255, cv::THRESH_BINARY_INV);


    //make the source image smaller so the blob detector works better
    resize(thresh, res, thresh.size() / 2);

    //add white border (blobs that end at the edge of the image are not detected)
    copyMakeBorder(res, border, BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, cv::BORDER_CONSTANT,
                   cv::Scalar(255, 255, 255));


    //Make keypoint object, detect blobs and put them in keypoints
    std::vector<cv::KeyPoint> keypoints;
    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);
    detector->detect(border, keypoints);

    //make a copy of the keypoints and aggregate them (copy is for development purposes)
    std::vector <cv::KeyPoint> original(keypoints);
    aggergatekeypoints(keypoints);

    //for development / visual purposes: draw keypoints on image
    if(visual) {
        drawKeypoints(border, keypoints, im_with_keypoints, cv::Scalar(0, 0, 255),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        drawKeypoints(border, original, keypoints_original, cv::Scalar(0, 0, 255),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    }


    //match keypoints to tracked persons
    for (int i = 0; i < keypoints.size(); i++) {
        if (keypoints[i].size > 0) {
            //search the current person
            bool found = false;
            for (int j = 0; j < tracker.size(); j++) {
                if (cv::KeyPoint::overlap(keypoints[i], tracker[j].last_keypoint) > 0) {
                    tracker[j].last_keypoint = keypoints[i];
                    tracker[j].misses = -1;
                    tracker[j].tracked_for++;
                    push_point_history(tracker[j]);
                    estimate_next(tracker[j]);
                    found = true;
                    break;
                }

            }
            //if person not found, add a new one
            if (!found) {
                struct person new_person;
                new_person.last_keypoint = keypoints[i];
                new_person.color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
                tracker.push_back(new_person);
                if(dev) std::cout << "added person as " << new_person.color << std::endl;
            }
        }
    }

    //points that were not found should get an estimate next point (interpolation)
    for (int i = 0; i < tracker.size(); i++) {
        if (tracker[i].misses == 0 && tracker[i].tracked_for > 2) {
            tracker[i].last_keypoint.pt += tracker[i].estimated;
            if(dev) std::cout << "tracker lost - using an estimate" << std::endl;
        }
    }

    //get rid of overlapping tracker ROI's (happens sometimes)
    for (int i = 0; i < tracker.size(); i++) {
        for (int j = i + 1; j < tracker.size(); j++) {
            eliminate_overlap(tracker[i], tracker[j]);
        }
    }

    //update tracked person misses and remove old persons.
    //SPOOKY SCARY ITERATORS
    border_size = border.size();
    update_trackers();






    //draw tracked persons (if dev)
    if(visual) {
        for (int i = 0; i < tracker.size(); i++) {
            rectangle(im_with_keypoints,
                      cv::Point(tracker[i].last_keypoint.pt.x - TRACK_REGION,
                                tracker[i].last_keypoint.pt.y - TRACK_REGION),
                      cv::Point(tracker[i].last_keypoint.pt.x + TRACK_REGION,
                                tracker[i].last_keypoint.pt.y + TRACK_REGION),
                      tracker[i].color, 2);
        }



        //devel imshow's
        putText(im_with_keypoints, "Number of people: " + std::to_string(tracker.size()),
                cv::Point(10, 20),
                CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1, cv::LINE_8, false);


        //cv::imshow("Detected", im_with_keypoints);
        cv::namedWindow("Input", 0);
        cv::namedWindow("Detected", 0);
        cv::imshow("Input", img);
        cv::imshow("Detected", im_with_keypoints);
        cv::waitKey(10);
    }

}


//turn development mode on or off
void depthpeopledetector::devel(bool option) {
    dev = option;
    if(dev) std::cout << "[depthpeopledetector] running in dev mode" << std::endl;
}

void depthpeopledetector::visualize(bool option) {
    visual = option;
}

void depthpeopledetector::update_trackers() {
    std::vector<struct person>::iterator pointer = tracker.begin(), end = tracker.end();

    while (pointer != end) {
        pointer->misses++;

        //if a person has hit their misses threshold, check if they moved from edge-to-edge
        //if so, add them to traversed_in or traversed_out (depending on last known location)
        if (pointer->misses > MISS_THRESH) {
            float ratio_x = (pointer->max.x - pointer->min.x) / img_roi.width;
            float ratio_y = (pointer->max.y - pointer->min.y) / img_roi.height;
            if(dev) {
                std::cout << std::endl << "*** removed a person ***" << std::endl;
                std::cout << "x-ratio: " << ratio_x << " :: y-ratio: " << ratio_y << std::endl;
                std::cout << "xminmax: " << pointer->min.x << " - " << pointer->max.x << std::endl;
                std::cout << "yminmax: " << pointer->min.y << " - " << pointer->max.y << std::endl << std::endl;
            }


            //We are only interested in the Y ratio. Use border.height
            //TODO: implement choice between x or y detection
            if (ratio_y > trans_thresh) {
                if (pointer->last_keypoint.pt.y > border_size.height / 2) {
                    if(dev) std::cout << "added to traversed_out" << std::endl;
                    traversed_out++;
                } else {
                    if(dev) std::cout << "added to traversed_in" << std::endl;
                    traversed_in++;
                }

            }

            //ultimately, remove person from tracked list
            tracker.erase(pointer);
        }
        pointer++;
    }
}

void depthpeopledetector::cleanup() {
    for(int i = 0; i < tracker.size(); i++) tracker[i].misses = MISS_THRESH;
    update_trackers();
}

int depthpeopledetector::results_in() {
    return traversed_in;
}

int depthpeopledetector::results_out() {
    return traversed_out;
}


bool depthpeopledetector::set_missthresh(int value) {
    if(value >= 0) {
        MISS_THRESH = value;
        return true;
    }
    else return false;
}

bool depthpeopledetector::set_trackingregion(int value) {
    if(value >= 0) {
        TRACK_REGION = value;
        return true;
    }
    else return false;
}

bool depthpeopledetector::set_bordersize(int value) {
    if(value >= 0) {
        BORDER_SIZE = value;
        return true;
    }
    else return false;
}

bool depthpeopledetector::set_transthresh(float value) {
    if(value >= 0 && value <= 1) {
        trans_thresh = value;
        return true;
    }
    else return false;
}

bool depthpeopledetector::set_imgroi(cv::Rect value) {
    img_roi = value;
    return true;
}
