#!/usr/bin/perl -w
#########################################################
#
#   xen_deploy: A sample script for cloning a Xen virtual
#   machine, creating a virtual floppy disk, and starting
#   it with the v-floppy attached. 
#
##########################################################
use Getopt::Long; # Parsing a command line (standard perl)

#
# Global Variables
#
my $VERBOSE = 1;
my @new_disk = ();

# Debug variables
my $DEBUG = 0;  
my $CONFIRM_ACTION = 0; # Set to 1 for interactive debug

#
# ARGV processing
#
if (@ARGV > 0){
    GetOptions( # Debug options
                'verbose' => \$VERBOSE, # Turn on more messages
                'confirm' => \$CONFIRM_ACTION, # Turn on more messages

                # Locations of Virtual Machine
                'sourceVMConfig=s'  => \$sourceVMConfig,   # config file
                'targetDirectory=s' => \$targetDirectory,  # $target directory
                
                # VM image customize information
                'vmname=s'  => \$vmname,    # $vmname
                'vcpus=s'=> \$numvcpus,  # $vcpus
                'memory=s'  => \$memory,    # $memory RAM
                'macaddr=s' => \$macaddr,   # $macaddr
                'xml=s'     => \$AP_FILE, #The AP file
                
                );
}else{
    print "Syntax error: Must have command line options... Example syntax:\n";
    print $0 . " -sourceVMConfig /etc/xen/vm/sles-10 -targetDirectory /etc/xen/images -vmname susecopy -memory 512 -vcpus 1 -xml XML.ap\n\n";
    die  "   try again...\n";
}

die "USER ERROR: -sourceVMConfig missing\n"  if not defined $sourceVMConfig;
die "USER ERROR: -targetDirectory missing\n" if not defined $targetDirectory;
die "USER ERROR: -vmname\n"                  if not defined $vmname;
die "USER ERROR: -vcpus missing\n"					 if not defined $numvcpus;
die "USER ERROR: -memory missing\n"          if not defined $memory;
die "USER ERROR: -xml missing\n"             if not defined $AP_FILE;

die "Please Use Full Path In Parameter\n"    if $sourceVMConfig !~ /^\//;
die "Please Use Full Path In Parameter\n"    if $targetDirectory !~ /^\//;

#
# Step 1. Make a directory that holds the new virtual machine image files. Add
#         an extension .0 .1 .2 .3 if necessary to get a unique location
#
#if (-e "${targetDirectory}/${vmname}"){
#    my $extension = 0;
#    while  (-e "${targetDirectory}/${vmname}_${extension}") {
#	$extension++;
#    }
#    $vmname = $vmname . '_' . $extension;
#}
#$full_targetDirectory      = "${targetDirectory}/${vmname}";
$full_targetDirectory = "${targetDirectory}";
$full_targetDirectory_temp = undef;

print "Step 1. Make a directory to hold virtual machine";
#&system_exec("Step 1. Make a directory to hold virtual machine",
#             "mkdir ${full_targetDirectory}", '^\w*$' );


#
# Step 2. RANDOM MAC SELECTION if MAC not specified. PLEASE BE ADVISED
#         that this avoids repeat MAC's by scanning the current hypervisor
#         only!
#
if (! defined $macaddr){
    my $randmac = sprintf("00:16:3e:%X:%X:%X", int(rand(255)), int(rand(255)), int(rand(255)));
    while (`grep -i $randmac /etc/xen/vm/*`){
        $randmac = sprintf("00:16:3e:%X:%X:%X", int(rand(255)), int(rand(255)), int(rand(255)));
    }
    $macaddr = $randmac;
}


#
# Step 3. Floppy disk creation and attach
#
#              dd if=/dev/zero of=/file bs=1k count=1440
#              losetup /dev/loop0 /file
#              mkfs -t ext2 /dev/loop0 100
#              mount -t ext2 /dev/loop0 /mnt
#               ...
#              umount /dev/loop0
#              losetup -d /dev/loop0
if (-e $AP_FILE){
    my $floppy = '/tmp/floppy.img';
    my $temp_mount = "/tmp/floppy_$$";
    
    &system_exec("Step 3a. Make a floppy disk",
                 "dd if=/dev/zero of=$floppy bs=4k count=16k");

    # Find an unused loop device, Linux has eight by default
    my $loopX;
    foreach(0..7){
        system("losetup /dev/loop$_");
        if ($?){
            $loopX = "loop$_";
            last;
        }
    }
    if (! defined $loopX){
        die "Can't find an open /dev/loop? loop [0..7] active\n";
    }

    
    &system_exec("Step 3b. Prepare the floppy for mounting",
                 "losetup /dev/${loopX} $floppy");

    
    my $fdiskstr="n\\np\\n1\\n\\n\\nw\\n";
    print "Step 3c. Put an ext2 file system on it";
    print `echo -e \"$fdiskstr\" | fdisk /dev/${loopX} 1>/dev/null 2>&1`;
    print `losetup -d /dev/${loopX}`;
    $loopX = "";
    foreach(0..7){
        system("losetup /dev/loop$_");
        if ($?){
            $loopX = "loop$_";
            last;
        }
    }
    if (! defined $loopX){
        die "Can't find an open /dev/loop? loop [0..7] active\n";
    }
    
    my $unit = `fdisk -ul $floppy 2>/dev/null | grep Units | awk '{i=NF}{print \$(i-1)}'`;
    $unit =~ s/\n$//;
    my $start = `fdisk -ul $floppy 2>/dev/null | grep $floppy"1" | awk '{print \$2}'`;
    $start =~ s/\n$//;
    my $end = `fdisk -ul $floppy 2>/dev/null | grep $floppy"1" | awk '{print \$3}'`;
    $end =~ s/\n$//;
    
    my $size= $end - $start + 1;
    $size= $size * $unit;
    my $partitionstart = $unit * $start;
    
    print "losetup -o $partitionstart -s $size /dev/${loopX} $floppy";
    print `losetup -o $partitionstart /dev/${loopX} $floppy`;

    print `mkfs -t ext2 /dev/${loopX} 1>/dev/null 2>&1`;
    &system_exec("Step 3d. Make a temp dir to mount it",
                 "mkdir $temp_mount");

    &system_exec("Step 3e. Mount the floppy to the $temp_mount location", 
                 "mount -t ext2 /dev/${loopX} $temp_mount");

    # Copy the ap file to the floppy disk 
    &system_exec("Step 3f. Copy the ap file into floppy",
                 "cp $AP_FILE ${temp_mount}/");
    &system_exec("Step 3g. Create a README in floppy",
                 "echo 'Created by xen_deploy.pl' > ${temp_mount}/README");
    # Unmount the floppy
    &system_exec("Step 3h. Unmount the floppy",
                 "umount /dev/${loopX}");
    &system_exec("Step 3i. Remove the loop device for floppy",
                 "losetup -d /dev/${loopX}");
    &system_exec("Step 3j. Move the floppy disk into the current directory",
                 "mv $floppy $full_targetDirectory ");
    &system_exec("Step 3k. Remove the temp dir used to mount it",
                 "rm -rf $temp_mount");
    push @new_disk, "'file:${full_targetDirectory}/floppy.img,hdb,w'";
}else{
    die "Can't locate [$AP_FILE] to copy to floppy disk?\n";
}



#
# Step 4. Copy VM disks from sourceVMConfig and prepare the xen config file
#          - Also add in the floppy disk we just created above
#
my $xenvm_path = '';
if ( defined $sourceVMConfig)
{
    # Open the Xen config and read it line by line
    
    my $in = "${sourceVMConfig}";
    $sourceVMConfig =~ /(.*)\//;
    my $out = "$full_targetDirectory/$vmname";
    open(INFILE,  "$in") or die "Can't open Xen config $in\n";
    open(OUTFILE, ">$out") or die "Can't create Xen config outfile $out\n";
    while (<INFILE>){
        if(/^memory/ and defined $memory){
            print OUTFILE "memory = $memory\n";
        }elsif(/^vcpus/ and defined $numvcpus){
            print OUTFILE "vcpus = $numvcpus\n";
        }elsif(/^vif/ ){
            # Substitute the MAC address with the new one.
            s/mac=(\w{1,2}:\w{1,2}:\w{1,2}:\w{1,2}:\w{1,2}:\w{1,2})/mac=${macaddr}/;
            print OUTFILE $_;
        }elsif(/^name/){
            print OUTFILE "name = '$vmname'\n";
        }elsif(/^disk/){
            # example: disk= [ 'file:/path/hda,hda,w', 'file:/path/hdb,hdb,r']
            # Match on everything inside brackets [*]
            $_ =~ /\[(.*)\]/;

            # Split based on quotes ['"] to get each disk info
            foreach my $item (split(/[\'\"]\s*,\s*[\'\"]/, $1 )){
                # Strip off leading and trailing quotes ['"] and space
                $item =~ s/[\'\"\s]//g;

                # Split out: path/to/disk/image, [hda|hdb|...], [r|m|w] permission
                if ($item =~ /file:(.*)\/(.*),(.*),(.*)/){
                    # Copy the VM
                    my $path=$1;
                    my $file=$2;
                    my $hdx=$3;
                    my $perm=$4;
                    push @new_disk, "'file:${full_targetDirectory}/${vmname}.img,$hdx,$perm'";
                    &system_exec("Copy image file [$file] to target location $full_targetDirectory",
                                 "cp ${path}/${file} ${full_targetDirectory}/${vmname}.img");
#                    if ( ("${full_targetDirectory}" ne "/etc/xen/images/${vmname}")
#                                                    and
#                         ("${full_targetDirectory}" ne "/var/lib/xen/images/${vmname}" )){
#                        &system_exec("Soft link to the target location $full_targetDirectory",
#                                     "ln -sf $full_targetDirectory/$vmname /etc/xen/vm/${vmname}");
#                    }else{
#                        print "${full_targetDirectory} is in the default directory.\n";
#                    }
                    
                }else{
                    die "Error: Xen file [$in] can't handle a [$item]. Quitting...\n";
                }
            }
            print OUTFILE "disk = [ " , join (",", @new_disk), " ]\n";
        }else{
            # Just print unrecognized stuff
            print OUTFILE $_;
        }
    }
    close(INFILE);
    close(OUTFILE);
    $xenvm_path = $out;
}else{
    die "Where is your source virtual machine on this hypervisor?\n";
}


#
# Step 5. Start the Virtual Machine
#
#if (-e $xenvm_path ){
#    &system_exec("Step 4. Start the Xen virtual machine using xm create",
#                 "xm create $xenvm_path");
#}else{
#    die "Error: No Xen config file was created at [$xenvm_path]\n";
#}




#
# Final Cleanup: Remove temp files if they exist
#
if (defined $full_targetDirectory_temp && -e $full_targetDirectory_temp){
    &system_exec("Final Cleanup. Remove temp files",
                 "rm -rf $full_targetDirectory_temp");
}

print "\n\n-- All done! --\n";
print " Type 'xm list' to see list of Xen VM's, and 'xm console $vmname' to connect\n";



######################################## 
#     Subroutines                      #
########################################

# system_exec
#   - small utility function for calling a system
#     command and checking for error response.
#
sub system_exec{
    my $description = shift;
    my $cmd = shift;
    my $validate = shift;
    my $res = '';
    
    print "$description\n";
    print " $cmd\n" if $VERBOSE;

    if ($CONFIRM_ACTION){
        print "(y|n|a|q) ?";
        my $resp = <>;
        die "Quit response." if $resp =~ /^q/;
        $CONFIRM_ACTION = undef if $resp =~ /^a/;
        return if $resp =~ /^n/;
        die "huh? don't understand $resp" unless $resp =~ /^(y|n|a|q)/;
    }
    #
    # Call the system command!
    #
    $res = `$cmd`;

    #
    # Die on error string found in $?
    #
    die "Failed $description: $? \n$res\n" if ($?);
    
    if (defined $validate){
        if ($res !~ /${validate}/){
            chomp $res;
            print "$res\n";
            die " ERROR: Failed $description -- '$validate'\n";
        }
    }
    return $res;
}

