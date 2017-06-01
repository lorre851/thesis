//
// Created by lorre851 on 22.02.17.
//

#include "stereotools.h"



stereotools::stereotools() {

}

int stereotools::load_config(string intrinsic_config, string extrinsic_config) {
    // reading intrinsic parameters
    fs = new FileStorage(intrinsic_config, FileStorage::READ);
    if(!fs -> isOpened()) {
        printf("Failed to open intrinsic file %s\n", intrinsic_config.c_str());
        has_config = false;
        return -1;
    }

    (*fs)["M1"] >> M1;
    (*fs)["D1"] >> D1;
    (*fs)["M2"] >> M2;
    (*fs)["D2"] >> D2;


    M1 *= scale;
    M2 *= scale;

    fs -> open(extrinsic_config, FileStorage::READ);
    if(!fs -> isOpened())
    {
        has_config = false;
        printf("Failed to open extrinsic file %s\n", extrinsic_config.c_str());
        return -2;
    }

    (*fs)["R"] >> R;
    (*fs)["T"] >> T;
    init_rectify_map = false;
    has_config = true;
    return 0;
}

/*
 *      TODO: pls fix this
 *
 */
Mat* stereotools::generate_depth_map(const Mat &img2_source, const Mat &img1_source) {
    
    if(!has_config) {
        cout << "[stereotools] <generate_depth_map> ERROR: no valid camera calibration was loaded!" << endl;
	return NULL;
    }
    else if(&img2_source == NULL || &img1_source == NULL) {
        cout << "[stereotools] <generate_depth_map> ERROR: invalid input images!" << endl;
        return NULL;
    }
    else if(img2_source.size() != img1_source.size()) {
	cout << "[stereotools] <generate_depth_map> ERROR: image size mismatch!" << endl;
    }


    


    
    Mat img1, img2;


    cvtColor(img1_source, img1, CV_BGR2GRAY);
    cvtColor(img2_source, img2, CV_BGR2GRAY);




    if(!init_rectify_map) {
        img_size = img1.size();

	//generate remapping matrices
        stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2);



        //syntax: cameraMatrix, distCoeffs, R, newCameraMatrix, size, m1type, map1 (out), map2 (out)
        initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
        initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);

	//set sbmg settings
	
	numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((img_size.width/8) + 15) & -16;
	cn = img1.channels();
	sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
	sgbm->setPreFilterCap(63);
	sgbm->setBlockSize(sgbmWinSize);
	sgbm->setP1(8*cn*sgbmWinSize*sgbmWinSize * 2);
	sgbm->setP2(32*cn*sgbmWinSize*sgbmWinSize * 2);
	sgbm->setMinDisparity(0);
	sgbm->setNumDisparities(numberOfDisparities);
	sgbm->setUniquenessRatio(10);
	sgbm->setSpeckleWindowSize(100);
	sgbm->setSpeckleRange(32);
	sgbm->setDisp12MaxDiff(1);
	sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);
	
	init_rectify_map = true;

	
    }



    Mat img1r(img1), img2r(img2);
    
    remap(img1, img1r, map11, map12, INTER_LINEAR);
    remap(img2, img2r, map21, map22, INTER_LINEAR);
    



   

    Mat disp;
    Mat *disp8 = new Mat();


    sgbm->compute(img1r, img2r, disp);


    disp.convertTo(*disp8, CV_8U, 255/(numberOfDisparities*16.));

    return disp8;


}

bool stereotools::validate_config() {
    return has_config;
}
