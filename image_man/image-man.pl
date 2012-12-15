#!/usr/bin/perl
#
# image repository managment
#

use Getopt::Std;

%options=();
getopts("arlci:n:m:y:p:x:", \%options);

if(defined $options{a}){
	if(! defined $options{i}){
		die "use -i\n";
	}
	if(! -e $options{i}){
		die "$options{i} not exist\n";
	}
	system "cp $options{i} /mnt/.";
	print "cp $options{i} /mnt/.\n";
}
elsif(defined $options{r}){
	if(! defined $options{i}){
		die "use -i\n";
	}
	if(! -e "/mnt/$options{i}"){
		die "/mnt/".$options{i}." not exist\n";
	}
	system "rm -f /mnt/$options{i}";
	print "rm $options{i}\n";
}
elsif(defined $options{l}){
	system "ls -l /home/share/template | grep img";
}
elsif(defined $options{c}){
	if(! defined $options{i}){
		die "use -i\n";
	}
	if(! defined $options{n}){
		die "use -n\n";
	}
	if(! defined $options{p}){
		die "use -p to define vcpus\n";
	}
	if(! defined $options{y}){
		die "use -y to define memory\n";
	}
	if(! defined $options{x}){
		die "use -x to define xml\n";
	}
	if(! -e "/mnt/$options{i}"){
		die "iamge $options{i} not exist\n";
	}
	if(-e "/mnt/$options{n}"){
		die "image $options{n} has been exist\n";
	}
	$cmd = "./xen_deploy.pl -sourceVMConfig /mnt/".$options{i};
	$cmd .= " -targetDirectory /mnt";
	$cmd .= " -vmname ".$options{n};
	$cmd .= " -macaddr ".$options{m} if defined $options{m};
	$cmd .= " -memory ".$options{y};
	$cmd .= " -vcpus ".$options{p};
	$cmd .= " -xml ".$options{x};
	print "$cmd\n";
	system "$cmd";
}
else{
	die "usage error\n"
}
