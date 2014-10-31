my @m = ();

my $HEADER = 0;

my $num = 0;
my $abs_num = 0;

if ($#ARGV > 0 && $ARGV[0] == "--header") {
	$abs_num = <STDIN>;
	$num = <STDIN>;
	chomp $num;
}


my $start = 1;

while(<STDIN>) {
	chomp;

	if ($_ eq "") {

		if ($start == 1) {
			$start = 0;
		}
		else {

			# print
			for my $i (1..$num) {
				print $m[$i][1];
				for my $j (2..$num) {
					print ",";
					print $m[$i][$j];
				}
				print "\n";
			}
			print "\n";
		}


		for my $i (1..$num) {
			$m[$i] = [];
			for my $j (1..$num) {
				$m[$i][$j] = 0;
			}
		}
	}
	else {
		chomp;
		my ($i, $j) =  split(",");
		if ($abs_num <= 0) {
			if ($i > $num) {
				$num = $i;
			}
			if ($j > $num) {
				$num = $j;
			}
		}
#		print "$i,$j\n";
		$m[$i][$j] = 1;
		$m[$j][$i] = 1;
	}
}

# print
for my $i (0..$num) {
	my $out = $m[$i][0];
	if ($out eq "") {
		$out = -1;
	}
	print $out;
	for my $j (1..$num) {
		print ",";
		my $out = $m[$i][$j];
		if ($out eq "") {
			$out = -1;
		}
		print $out;
	}
	print "\n";
}
