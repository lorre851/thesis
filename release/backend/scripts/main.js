
//new functions

function confirmRemoveUser() {
	var res = confirm("Are you sure you want to remove this user? This cannot be undone!");
	return res;
}

function confirmDeactivateUser() {
	var res = confirm("Are you sure you want to deactivate this user?");
	return res;
}

function confirmActivateUser() {
	var res = confirm("Are you sure you want to activate this user?");
	return res;
}

function confirmDeleteDomain() {
	var res = confirm("Are you sure you want to remove this domain? This cannot be undone!");
	return res;
}

function manageDomains(userid) {
	var domains = prompt("Please enter the new domains for user '"+userid+"'. Separate each domain with a ';'. Old domain configuration will be overwritten!");
	if(domains != null && domains != '') window.location = "index.php?&manage=Users&page=Manage%20domains&action=domains&target="+userid+"&domains="+domains;
}

function resetDomains(userid) {
	var domains = confirm("Are you sure you want to reset the domain list of user '"+userid+"' to default configuration?");
	if(domains) window.location = "index.php?&manage=Users&page=Manage%20domains&action=reset&target="+userid;
}


function confirm_remove_action(id) {
	$("#target").val(id);
	$(".actionv").fadeIn();
}


function confirmsimulate(userid) {
	$("#userid").val(userid);
	$(".simulate_login").fadeIn();
}

function changePass(userid) {
	$("#target").val(userid);
	$(".changepass").fadeIn();
}

function updatePluginDomain(pluginid, domain) {
	$("#plugin_id").val(pluginid);
	$("#plugin_domains").val(domain);
	$(".pluginpriv").fadeIn();
}

function displayWarning(alert, action) {
	$("#overlay_warning").html(alert);
	$("#overlay_continue").attr('href', action);
	$(".continue").fadeIn();
}


function toggleMenu() {
	$("#main_menu_items").fadeOut("fast");
	if($("#navigator").css("display") == "none") $("#navigator").fadeIn("fast");
	else $("#navigator").fadeOut("fast");
}

function toggleMain() {
	$("#navigator").fadeOut("fast");
	if($("#main_menu_items").css("display") == "none") $("#main_menu_items").fadeIn("fast");
	else $("#main_menu_items").fadeOut("fast");
}


$(document).ready(function() {
	$(window).on('resize', function(){
		  var win = $(this); //this = window
		  if (win.width() > 700) { 
			$("#navigator").fadeIn("fast");
			$("#main_menu_items").css("display", "inline-block");
		  }
		  else {
			$("#navigator").fadeOut("fast");
			$("#main_menu_items").fadeOut("fast");
		  }
		  
	});
});
