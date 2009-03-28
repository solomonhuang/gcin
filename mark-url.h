<?php

function mark_url_cb($mat)
{
// var_dump($mat);

$m1 = $mat[1];
$m2 = $mat[2];
$m6 = $mat[6];
$m9 = $mat[9];

$url = $m2;
if ($m6)
  $url.=preg_replace('!&amp;!', '&', $m6);

$ma="$m1<a href='$url' target='_blank'>$m2$m6</a>$m9";

return $ma;
}

function mark_url($str)
{
return preg_replace_callback('!([^\'>"]|^)((http|ftp|https)://[\.\w/\-]{1,80}(\?(\w+=[%\w/\.~]+)+|))((&amp;(\w+=[%\w/\.]+))+|)([^\'<"]|&.{1,8};|$)!',
 'mark_url_cb', $str);
}

?>
