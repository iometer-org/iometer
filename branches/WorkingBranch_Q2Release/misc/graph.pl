#!/usr/bin/perl

# graphing script to use if Access isn't an option.  :)
# written by Ian Brown for the iometer folks.
# Version 1.0.0

#if using windows (activestate perl), you must 'ppm install' these dependencies:
# Time-Local
# DBD-Chart
# DBI
# http://theoryx5.uwinnipeg.ca/ppms/GD.ppd
# http://theoryx5.uwinnipeg.ca/ppms/GDTextUtil.ppd

#for the *nix folk:
#perl -MCPAN -eshell
#  install DBD::Chart
# or find the rpm/package for your distro

# todo:
# write a config file that this script can parse instead of asking for user input

use warnings;
use strict;

use DBI;
use DBD::Chart;

sub HarvestChoicesFromCSVFile($);
sub HarvestDataFromCSVFile($$$);
sub CreateTable($);
sub InsertData($$);
sub UpdateData($$$);
sub GraphData($$$$);

if (@ARGV < 2)
{
   die("Usage: graph.pl file1.csv [file2.csv file3.csv] " .
       "output_graphic_filename_without_extension\n");
}
if (@ARGV > 29)
{
   die("Sorry, this program can only generate 28 lines per graph due to" .
       " the limitation of color choices.\n");
}

my @filenames = @ARGV;
my $graphicFilename = pop(@filenames);

my @filenameTitles;

for (my $i = 0; $i < @filenames; $i++)
{
   if (! -f $filenames[$i])
   {
      die "Filename: $filenames[$i] not found.\n";
   }
   else
   {
      print "Enter in a title for $filenames[$i] (Default is name itself): ";
      $filenameTitles[$i] = <STDIN>;
      chomp($filenameTitles[$i]);
      if ($filenameTitles[$i] eq "")
      {
         $filenameTitles[$i] = $filenames[$i];
         $filenameTitles[$i] =~ s/.csv$//i;
      }
      $filenameTitles[$i] = uc($filenameTitles[$i]);
      $filenameTitles[$i] =~ tr/[A-Z][0-9]/_/c;
   }
}

unshift(@filenameTitles, "placeholder");
my @choices = @{HarvestChoicesFromCSVFile($filenames[0])};
my $displacement = 2;
for (my $i = 1 + $displacement; $i < @choices; $i++)
{
   print $i - $displacement . ": $choices[$i]\n";   
}

my $yColumn;

do
{
   print "Which # for the y (vertical) column: ";
   $yColumn = <STDIN>;
   chomp $yColumn;
} while ($yColumn !~ m/^\d+$/);

my $xColumn;

do
{
   print "Which # for the x (horizontal) column: ";
   $xColumn = <STDIN>;
   chomp $xColumn;
} while ($xColumn !~ m/^\d+$/);

$yColumn+=$displacement;
$xColumn+=$displacement;

print "What would you like the y-axis to be titled? (Enter for default)\n";
print "[" . $choices[$yColumn] . "]: ";
my $yAxisTitle = <STDIN>;
chomp $yAxisTitle;
if ($yAxisTitle =~ m/^$/)
{
   $yAxisTitle = $choices[$yColumn];
}

print "What would you like the x-axis to be titled? (Enter for default)\n";
print "[" . $choices[$xColumn] . "]: ";
my $xAxisTitle = <STDIN>;
chomp $xAxisTitle;
if ($xAxisTitle =~ m/^$/)
{
   $xAxisTitle = $choices[$xColumn];
}

print "What would you like the title to say? (Enter for default)\n";
print "[" . $yAxisTitle . " vs. " . $xAxisTitle . "]: ";
my $title = <STDIN>;
chomp $title;
if ($title =~ m/^$/)
{
   $title = $yAxisTitle . " vs. " . $xAxisTitle;
}

CreateTable(\@filenameTitles);

my $lineCount = @filenames;
my $filename = shift(@filenames);
my $hashRef = HarvestDataFromCSVFile($filename, $yColumn, $xColumn);
InsertData($hashRef, $lineCount + 1);
for (my $i = 0; $i < @filenames; $i++)
{
   $hashRef = HarvestDataFromCSVFile($filenames[$i], $yColumn, $xColumn);
   UpdateData($hashRef, $i+2, \@filenameTitles);
}

my $fileRef = GraphData($yAxisTitle, $xAxisTitle, $lineCount, $title);
open FILE, ">" . $graphicFilename . ".png";
binmode(FILE);
print FILE $$fileRef;
close FILE;


sub HarvestChoicesFromCSVFile($)
{
   my $filename = shift;
   my @choices;
   open FILE, "<$filename";
   while (<FILE>)
   {
      if ($_ =~ m/^'Results/)
      {
         my $choices = <FILE>;
         @choices = split(/,/, $choices);
         $choices[0] =~ s/\'//;
         last;
      }
   }
   close FILE;
   return \@choices;
}
sub HarvestDataFromCSVFile($$$)
{
   my $filename = shift;
   my $yColumn = shift;
   my $xColumn = shift;
   my %hash;
   open FILE, "<$filename";
   while (<FILE>)
   {
      if ($_ =~ m/^ALL/)
      {
         my @data = split(/,/, $_);
         push (@{$hash{'xColumn'}}, $data[$yColumn]);
         push (@{$hash{'yColumn'}}, $data[$xColumn]);
      }
   }
   close FILE;
   return \%hash;
}

BEGIN
{
   my $dbh = DBI->connect('dbi:Chart:');

   sub CreateTable($)
   {
      my $filenameTitlesRef = shift;
      
      my @arrayOfFields;
      foreach my $title (@{$filenameTitlesRef})
      {
         push(@arrayOfFields, "$title DECIMAL");
      }
      my $listOfFields = join(", ", @arrayOfFields);
      my $createTableSql = "CREATE TABLE line ($listOfFields)";
      my $createTable = $dbh->prepare($createTableSql) or die
         "Couldn't prepare createTable.  This is almost certainly due to the filename title(s) given.\n" .
         "SQL COMMAND: $createTableSql\n";   
      $createTable->execute();
   }
   
   sub InsertData($$)
   {
      my $hashRef = shift;
      my $count = shift;
      my $valuesQuery = "?, ?";
      for(my $i = 2; $i < $count; $i++)
      {
         $valuesQuery .= ", NULL"
      }
      my @yColumn = @{$hashRef->{'xColumn'}};
      my @xColumn = @{$hashRef->{'yColumn'}};
      if (@yColumn != @xColumn)
      {
         die("xColumn doesn't have the same elements as yColumn.  Corrupt csv?\n");
      }
      my $sqlString = "INSERT INTO line VALUES($valuesQuery)";
      my $insertData = $dbh->prepare( $sqlString)
         or die "Couldn't prepare insertData.  This normally happens when there was a problem creating the table.\n" .
         "SQL COMMAND: $sqlString\n";       
      for (my $i = 0; $i < @yColumn; $i++)
      {
         $insertData->execute($xColumn[$i], $yColumn[$i]);
      }
   }
   
   sub UpdateData($$$)
   {
      my $hashRef = shift;
      my $placeHolder = shift;
      my $filenameTitlesRef = shift;
      my @titles = @{$filenameTitlesRef};
      my @yColumn = @{$hashRef->{'xColumn'}};
      my @xColumn = @{$hashRef->{'yColumn'}};
      my $sqlQuery = "UPDATE line set $titles[$placeHolder] = ? where $titles[0] = ?";
      my $updateData = $dbh->prepare($sqlQuery)
         or die "Couldn't prepare updateData.  This normally happens when there was a problem creating the table.\n" .
         "SQL COMMAND: $sqlQuery\n";
      for (my $i = 0; $i < @yColumn; $i++)
      {
         $updateData->execute($yColumn[$i], $xColumn[$i]);
      }      
   }
   
   sub GraphData($$$$)
   {
      my $yColumnName = shift;
      my $xColumnName = shift;
      my $numOfLines = shift;
      my $title = shift;
      my $buf;
      my @colors = ('yellow', 'blue', 'red', 'gray', 'dgray', 'black', 
                     'lblue', 'dblue', 'gold', 'lyellow', 'dyellow',
                    'lgreen', 'green', 'dgreen', 'lred', 'dred', 'lgray',
                    'lpurple', 'purple', 'dpurple', 'lorange', 'orange',
                    'pink', 'dpink', 'marine', 'cyan', 'lbrown', 'dbrown');
      
      my $colors;
      for (my $i = 0; $i < $numOfLines; $i++)
      {
         $colors .= $colors[$i] . ", ";
      }
      chop($colors);
      chop($colors);
      my $graphSql ="SELECT LINEGRAPH FROM line
        WHERE WIDTH=800 AND HEIGHT=600 AND X_AXIS= ? AND
        Y_AXIS= ? AND COLOR in ($colors) AND SHAPE='fillcircle' AND
        SHOWGRID=1 AND SHOWVALUES=0 AND BACKGROUND='white' AND LINEWIDTH=3 AND
        TITLE=?";
     my $graphData = $dbh->prepare($graphSql)
      or die ("Couldn't prepare graphData.  This normally happens when there was a problem inserting the data.\n" .
              "SQL COMMAND: $graphSql\n"); 
      $graphData->execute($xColumnName, $yColumnName, $title);
      $graphData->bind_columns(\$buf);
      $graphData->fetch();
      return \$buf;
   }
}


