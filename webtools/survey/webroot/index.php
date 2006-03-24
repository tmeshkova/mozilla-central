<?php
/**
 * Uninstall survey.
 * @package survey
 * @subpackage docs
 */
$intends = $app->getIntends();
$issues = $app->getIssues();



/**
 * If the user has submitted, process and complete the transaction.
 */
if (!empty($_POST['submit'])) {

// Clean inputs.  Yes, I know, it should probably be done in the DBI.
// We're not going to be validation nazis here, since the form is optional.
$sql = array();

// The easy stuff.
$sql['product'] = !empty($_POST['product'])?mysql_real_escape_string($_POST['product']):null;
$sql['useragent'] = !empty($_POST['useragent'])?mysql_real_escape_string($_POST['useragent']):null;
$sql['http_user_agent'] = mysql_real_escape_string($_SERVER['HTTP_USER_AGENT']);
$sql['intend_id'] = !empty($_POST['intend_id'])?mysql_real_escape_string($_POST['intend_id']):null;
$sql['intend_text'] = !empty($_POST['intend_text'])?mysql_real_escape_string($_POST['intend_text']):null;
$sql['comments'] = !empty($_POST['comments'])?mysql_real_escape_string($_POST['comments']):null;

// For each issue, we need to log the other text.
$sql['issue_id'] = array();
if (!empty($_POST['issue_id']) && is_array($_POST['issue_id'])) {
    foreach ($_POST['issue_id'] as $issue_id) {
        $sql['issue_id'][mysql_real_escape_string($issue_id)] = !empty($_POST[$issue_id.'_text'])?mysql_real_escape_string($_POST[$issue_id.'_text']):'';
    }
}

// Result record.
$query = "
    INSERT INTO
        result(intend_id, intend_text, product, useragent, http_user_agent, comments, date_submitted)
    VALUES(
        '{$sql['intend_id']}', '{$sql['intend_text']}', '{$sql['product']}', '{$sql['useragent']}', '{$sql['http_user_agent']}', '{$sql['comments']}', NOW()
    );\n
";
$db->query($query, SQL_NONE);

if (!empty($sql['issue_id']) && count($sql['issue_id']) > 0) {
    foreach ($sql['issue_id'] as $id => $text) {
        $db->query("INSERT INTO issue_result_map() VALUES(LAST_INSERT_ID(), '{$id}', '{$text}')", SQL_NONE);
    }
}

// Redirect to thank you page, and we're done.
require_once('./thanks.php');
exit;



/**
 * If we haven't submitted, show the form by default.
 */
} else {
require_once(HEADER);
echo '<form action="'.$_SERVER['PHP_SELF'].'" method="post" id="surveyform">';

// Create intend block.
echo '<h2>How did you intend to use Firefox when you installed it?</h2>';
echo '<ul class="survey">';
foreach ($intends as $id=>$text) {
    echo '<li><input type="radio" name="intend_id" id="int'.$id.'" value="'.$id.'" /> <label for="int'.$id.'">'.$text.'</label></li>';
}
echo '<li><label for="int0"><input type="radio" name="intend_id" id="int0" value="0"/> Other, please specify:</label> <input type="text" name="intend_text" id="intother" /></li>';
echo '</ul>';

// Create issue block.
echo '<h2>Why did you uninstall Firefox? (select all that apply)</h2>';
echo '<ul class="survey">';
foreach ($issues as $id=>$text) {
    echo '<li><label for="iss'.$id.'"> <input type="checkbox" name="issue_id[]" id="iss'.$id.'" value="'.$id.'" />'.$text.'</label></li>';
}
echo '</ul>';

echo '<h2>How can we improve Firefox?</h2>';
echo '<p>Please share your ideas, suggestions or details about any issues below.</p>';
echo '<div><textarea name="comments" rows="7" cols="60"></textarea></div>';

echo '<input type="hidden" name="product" value="'.htmlentities(!empty($_GET['product'])?$_GET['product']:null).'"/>';
echo '<input type="hidden" name="useragent" value="'.htmlentities(!empty($_GET['useragent'])?$_GET['useragent']:null).'"/>';

echo '<div><input name="submit" type="submit" class="submit" value="Submit &raquo;"/></div>';
echo '</form>';
require_once(FOOTER);
}
?>
