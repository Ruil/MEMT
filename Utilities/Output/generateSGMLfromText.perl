#!/usr/local/bin/perl5
#
# This program is used to creat the SGML file from a plain text file (the hypothesis generated by MT systems)
# using the template of the source SGML
#
# Assuming the order of the source SGML is the same as the hypothesis file
#
# Note: this is all for NIST SGML version 09
# Use convert8to9.perl to convert from SGML version 8 to v9
#
# By Joy, joy@cs.cmu.edu
# Mar 06, 2003
#
# Usage:
#	perl5 generateSGMLfromText.perl textFile sysId srcv.09.sgm > hyp.sgm
#

#check parameters
if($#ARGV<2){
	print STDERR "\n----------------------------------------------";
	print STDERR "\nUsage:";
	print STDERR "\n\tperl5 generateSGMLfromText.perl textFile sysId src.sgm > hyp.sgm";
	print STDERR "\n----------------------------------------------\n";
	
	exit;	
}

open HypFile, $ARGV[0];
$sysId = $ARGV[1];
open SrcFile, $ARGV[2];

@hypSegs = <HypFile>;

print STDERR "Hypothesis file has ",$#hypSegs+1," lines.\n";

$srcLines=0;
while(<SrcFile>){
	#version 9: 
	#src: <srcset setid="mt02_chinese_evlset_v0" srclang="Chinese">
	#tst: <tstset setid="[tst-set-name]" srclang="[src-lang]" trglang="[tgt-lang]">
	s/<srcset setid=\"([^\"]+)\" srclang=[\"]{0,1}([^\"]+)[\"]{0,1}>/<tstset setid=\"$1\" srclang=\"$2\" trglang=\"English\">/;
	s/<\/srcset>/<\/tstset>/;
	
	#Doc tag, add system id
	#<DOC docid="XIN20020317.0076" sysid="E01">
	#s/<DOC docid=\"([^\"]+)\">/<DOC docid=\"$1\" sysid=\"$sysId\">/;

	#<DOC docid="AFA20040101.0510" GENRE="news">
	s/<DOC docid=\"([^\"]+)\"(.*)>/<DOC docid=\"$1\" sysid=\"$sysId\"$2>/i;

	#replacing the source text with the hypothesis
	if(/<seg id=/){
		$thisTrans = $hypSegs[$srcLines];
		$thisTrans =~s/\n+\Z//;
#		s/<seg id=([0-9]+)>.+<\/seg>/<seg id=$1> $thisTrans <\/seg>/;
		s/<seg id=(\"?[^>\"]+\"?)>.+<\/seg>/<seg id=$1> $thisTrans <\/seg>/;
		$srcLines++;
	}
	
	print $_;
}

print STDERR "$srcLines in the src SGML file\n";