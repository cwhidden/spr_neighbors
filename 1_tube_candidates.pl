my $offset = 0;

if ($#ARGV >= 0) {
	$offset = $ARGV[0];
}

my $tree1=<STDIN>;
chomp $tree1;
my $tree2=<STDIN>;
chomp $tree2;
my $dist=`(echo "$tree1"; echo "$tree2") | rspr | grep 'total exact' | grep -o '[0-9]\\+'`;

# max distance from either tree
my $max_dist = int($dist / 2) + 2 + $offset;

# k-neighbors of tree1
print `(
		echo "$tree1" | ./spr_neighbors -k $max_dist;
		echo "$tree2" | ./spr_neighbors -k $max_dist;
		) | ./normalize | sort | uniq`;

# k-neighbors of tree2



