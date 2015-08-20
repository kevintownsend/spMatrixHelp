xst : pattern_decoder.prj
	echo "run -ifn pattern_decoder.prj -ifmt mixed -top pattern_decoder -ofn pattern_decoder.ngc -iobuf NO -bufg 0 -iob false -ofmt NGC -p xc5vlx330-2 -opt_mode Speed -opt_level 1 -vlgincdir common" | xst

pattern_decoder.prj :
	echo "verilog work pattern_decoder.v" > pattern_decoder.prj

cleanXst :
	rm -rf xst *.prj
