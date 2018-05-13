<?php
function assoc(&$m, $word)
{
	if(array_key_exists($word, $m))
	{
		$m[$word]++;
	}
	else
	{
		$m[$word] = 1;
	}
}

function benchmark()
{
	$f = fopen("words.txt", "r");
	$buffer = '';
	$m = array();
	$i = 0;

	while(!feof($f))
	{
		$content = fread($f, 81920);

		$buffer .= $content;

		$left = 0;
		$right = 0;

		while($left < strlen($buffer))
		{
			$right = strpos($buffer, ' ', $left);

			if($right === FALSE)
			{
				break;
			}

			$word = substr($buffer, $left, $right - $left);

			assoc($m, $word);

			$left = ++$right;
		}

		if($left > 0)
		{
			$buffer = substr($buffer, $left);
		}
	}

	fclose($f);

	if(strlen($buffer) > 0)
	{
		assoc($m, $buffer);
	}

	echo count($m).PHP_EOL;

	$sum = 0;

	foreach($m as $key => $value)
	{
		$sum += $value;
	}

	echo $sum.PHP_EOL;
}

$start = microtime(true);

benchmark();

$elapsed = microtime(true) - $start;

echo $elapsed.PHP_EOL;
?>
