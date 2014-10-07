my $tree1=<>;
chomp $tree1;
my $tree2=<>;
chomp $tree2;
my $dist=`(echo "$tree1"; echo "$tree2") | rspr | grep 'total exact' | grep -o '[0-9]\\+'`;

# max distance from either tree
my $max_dist = int($dist / 2) + 2;

# k-neighbors of tree1
print `(
		echo "$tree1" | ./spr_neighbors -k $max_dist;
		echo "$tree2" | ./spr_neighbors -k $max_dist;
		) | sort | uniq`;

# k-neighbors of tree2



