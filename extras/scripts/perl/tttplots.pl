#!/usr/bin/perl
##      ---------------------------------------------------------------
##
##      tttplots: A Perl program to ...
##
##      authors: Renata M. Aiex and Mauricio G. C. Resende
##
##      ---------------------------------------------------------------

##      ----------------------------------------------------------------
##      Input network and spec file names.
##      ----------------------------------------------------------------
	$datafilethere=0;
	while ($ARGV[0]) {
		if ($ARGV[0] eq "-f") {
			shift(@ARGV);
			$filename = $ARGV[0];
			$datafilename = $filename . ".dat";
			$datafilethere=1;
			shift;
		}

	}
	if ($datafilethere == 0) {
		die "Error, data file missing. \nUsage: perl tttplots.pl -f datafile.dat -o outputfile.out \n";
	}

##      ----------------------------------------------------------------
#	Name output files.
##      ----------------------------------------------------------------
	$emp_lin_filename = $filename . "-el" . ".dat";
	$the_lin_filename = $filename . "-tl" . ".dat";
	$up_lin_filename = $filename . "-ul" . ".dat";
	$lo_lin_filename = $filename . "-ll" . ".dat";
	$gpl_lin_filename = $filename . "-qq" . ".gpl";
	$ps_lin_filename = $filename . "-qq" . ".ps";
	$emp_exp_filename = $filename . "-ee" . ".dat";
	$the_exp_filename = $filename . "-te" . ".dat";
	$gpl_exp_filename = $filename . "-exp" . ".gpl";
	$ps_exp_filename = $filename . "-exp" . ".ps";

##      ----------------------------------------------------------------
#	Open data file.
##      ----------------------------------------------------------------
	open (DATFILE,$datafilename) || die "Cannot open file: $datafilename \n";

##      ----------------------------------------------------------------
#	Read data file.
##      ----------------------------------------------------------------
	$n=0;
	while ($line = <DATFILE>){
		chomp($line);
		@field = split(/\s+/,$line);

		$nfields=0;                 #
		foreach $fld (@field){      # count number of fields
			$nfields++;         #
		}                           #

		if ($nfields != 1){
			die "Number of fields in data file must be 1 \n";
		}
		$time_value[$n] = $field[0];
		$n++;
	}
	close (DATFILE);

##      ----------------------------------------------------------------
#	Sort times.
##      ----------------------------------------------------------------
	@sorted_time_value = sort { $a <=> $b } @time_value;

	print "\@--------------------------------------------------------@\n";
	print " tttplots: TIME TO TARGET (TTT) DISTRIBUTION PLOTS                         \n\n";
	print " Input data set > \n\n";
	print "    data file   : $datafilename \n\n";
	print "    data points : $n \n";
	print "    max value   : $sorted_time_value[$n-1] \n";
        $avg=0;
	for ($k=0; $k < $n; $k++){
		$avg=$avg + $sorted_time_value[$k];
	}
	$avg=$avg/$n;
	print "    avg value   : $avg \n";
	print "    min value   : $sorted_time_value[0] \n\n";

##      ----------------------------------------------------------------
#	Compute probabilities.
##      ----------------------------------------------------------------
	$nn = 0;
        $np1=$n+1;
	while ($nn < $n){
                $prob[$nn] = $nn + .5;
                $prob[$nn] = $prob[$nn] / $np1;
		$nn++;
	}

##	----------------------------------------------------------------------
#	Compute parameters
##	----------------------------------------------------------------------

	$fq = ($np1 * .25);
	$tq = ($np1 * .75);
	$fq = int($np1 * .25);
	$tq = int($np1 * .75);

        $y = $prob[$fq];
        $zl = $sorted_time_value[$fq];
        $ql = -log(1-$y);
        $y = $prob[$tq];
        $zu = $sorted_time_value[$tq];
        $qu = -log(1-$y);

        $lambda = ($zu - $zl)/($qu - $ql);
        $mu = $zl - ($lambda * $ql);

        print " Estimated parameters (theoretical shifted exponential distribution) > \n\n";
        print "    shift (mu)         : $mu \n";
        print "    std. dev. (lambda) : $lambda \n";
        $shifted_mean = $mu+$lambda;
        print "    mean (shifted)     : $shifted_mean  \n";
#
#	----------------------------------------------------------------------
#	Compute theoretical plot (400 points)
#	----------------------------------------------------------------------
        $tmax = $sorted_time_value[$n-1];
        $inv_lambda = 1/$lambda;
        $eps = $tmax/400;
	$nn = 1;
	while ($nn <= 400){
                $theory_t[$nn-1]= $eps * $nn;
                $theory_p[$nn-1] = 1-exp(-$inv_lambda*($eps * $nn - $mu));
		$nn++;
	}

#
#	----------------------------------------------------------------------
#	Compute theoretical time values
#	----------------------------------------------------------------------
	$nn = 0;
	while ($nn < $n){
                $theoretical_time[$nn] = -log(1-$prob[$nn]);
		$nn++;
	}

#
#	----------------------------------------------------------------------
#	Compute qqplot line, lower and upper error lines
#	----------------------------------------------------------------------
	$nn = 0;
	while ($nn < $n){
                $pi = $prob[$nn];
                $x[$nn] = -log(1-$pi);
                $qq_err[$nn] = $lambda * $x[$nn] + $mu;
                $dev = $lambda  * (sqrt($pi/((1-$pi)*$np1)));
                $lo_error_point[$nn] = $qq_err[$nn] - $dev;
                $up_error_point[$nn] = $qq_err[$nn] + $dev;
		$nn++;
	}

#
#	----------------------------------------------------------------------
#	Write output files ...
#	----------------------------------------------------------------------
	open (EMP_LIN_FILE,">$emp_lin_filename") ||
                    die "Cannot open file: $emp_lin_filename \n";
	open (UP_LIN_FILE,">$up_lin_filename") ||
                    die "Cannot open file: $up_lin_filename \n";
	open (LO_LIN_FILE,">$lo_lin_filename") ||
                    die "Cannot open file: $lo_lin_filename \n";
	open (EMP_EXP_FILE,">$emp_exp_filename") ||
                    die "Cannot open file: $emp_exp_filename \n";
	open (THE_LIN_FILE,">$the_lin_filename") ||
                    die "Cannot open file: $the_lin_filename \n";

	$nn = 0;
	while ($nn < $n){
                print EMP_EXP_FILE "$sorted_time_value[$nn] $prob[$nn] \n";
                print EMP_LIN_FILE "$theoretical_time[$nn] $sorted_time_value[$nn] \n";
                print LO_LIN_FILE "$x[$nn]  $lo_error_point[$nn] \n";
                print UP_LIN_FILE "$x[$nn]  $up_error_point[$nn] \n";
                print THE_LIN_FILE "$x[$nn]  $qq_err[$nn] \n";
		$nn++;
	}
	close (EMP_EXP_FILE);

#
#	----------------------------------------------------------------------
#	Theoretical exp file
#	----------------------------------------------------------------------
#
	open (THE_EXP_FILE,">$the_exp_filename") ||
                die "Cannot open file: $the_exp_filename \n";
	$nn = 0;
	while ($nn < 400){
		print THE_EXP_FILE "$theory_t[$nn] $theory_p[$nn] \n";
		$nn++;
	}

#
#	----------------------------------------------------------------------
#	Create qqplot gnuplot file.
#	----------------------------------------------------------------------
#
	open (GPL_LIN_FILE,">$gpl_lin_filename") ||
                       die "Cannot open file: $gpl_lin_filename \n";
	print GPL_LIN_FILE <<EOF;
		set xlabel \'exponential quantiles\'
		set size ratio 1
		set ylabel \'measured times\'
		set key right bottom
		set title \'$filename\'
		set terminal postscript color \'Helvetica\'
		set output \'$ps_lin_filename\'
		plot "$emp_lin_filename" t "empirical" w points, "$the_lin_filename" t "estimated" with lines ls 3, "$up_lin_filename" t "+1 std dev range" w lines ls 4, "$lo_lin_filename" t "-1 std dev range" w lines ls 4
		quit
EOF

#
#	----------------------------------------------------------------------
#	Create qqplot postscript graphic file.
#	----------------------------------------------------------------------
#
	open (PS_EXP_FILE,">$ps_exp_filename") ||
                     die "Cannot open file: $ps_exp_filename \n";
	system("gnuplot $gpl_lin_filename") == 0 ||
                     die "gnuplot (needed for plotting) not found \n";
;

#
#	----------------------------------------------------------------------
#	Create empirical-theoretical distributions gnuplot file
#	----------------------------------------------------------------------
#
	open (GPL_EXP_FILE,">$gpl_exp_filename") ||
                die "Cannot open file: $gpl_exp_filename \n";
	print GPL_EXP_FILE <<EOF;
		set xlabel \'time to target solution\'
		set size ratio 1
		set ylabel \'cumulative probability\'
                set yrange [0:1]
		set key right bottom
                set grid
		set title \'$filename\'
		set terminal postscript color \'Helvetica\'
		set output \'$ps_exp_filename\'
		plot "$emp_exp_filename" t "empirical" w points, "$the_exp_filename" t "theoretical" w lines ls 3
		quit
EOF

#
#	----------------------------------------------------------------------
#	Create empirical-theoretical distributions postscript graphic file
#	----------------------------------------------------------------------
#
	open (PS_EXP_FILE,">$ps_exp_filename") ||
                 die "Cannot open file: $ps_exp_filename \n";
	system("gnuplot $gpl_exp_filename") == 0 ||
                     die "gnuplot (needed for plotting) not found \n";

#
#	----------------------------------------------------------------------
#       End of program.
#	----------------------------------------------------------------------
#
        print "\n Output files > \n\n";
        print "    empirical exponential distribution data  : $emp_exp_filename  \n";
        print "    theoretical exponential distribution data: $the_exp_filename  \n";
        print "    empirical qq-plot data                   : $emp_lin_filename\n";
        print "    theoretical qq-plot data                 : $the_lin_filename\n";
        print "    theoretical upper 1 std dev qq-plot data : $up_lin_filename\n";
        print "    theoretical lower 1 std dev qq-plot data : $lo_lin_filename\n";
        print "    theor. vs empir. ttt plot gnuplot file   : $gpl_exp_filename\n";
        print "    theor. vs empir. qq-plot gnuplot file    : $gpl_lin_filename\n";
        print "    theor. vs empir. ttt plot postscript file: $ps_exp_filename\n";
        print "    theor. vs empir. qq-plot postscript file : $ps_lin_filename\n";
	print "\n DONE \n";
	print "\@--------------------------------------------------------@\n";
        print "\n";
