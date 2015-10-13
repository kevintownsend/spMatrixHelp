

all : checkVsim

vsim_pattern_decoder = work/pattern_decoder

vsim_pattern_decoder_tb = work/pattern_decoder_tb

vsim_asymmetric_distributed_ram = work/asymmetric_distributed_ram

vsim_asymmetric_fifo = work/asymmetric_fifo

vsim_std_fifo = work/std_fifo

work :
	vlib work

$(vsim_pattern_decoder) : pattern_decoder.v work
	vlog pattern_decoder.v +incdir+./common

$(vsim_pattern_decoder_tb) : pattern_decoder_tb.v work
	vlog pattern_decoder_tb.v +incdir+./common

$(vsim_std_fifo) : std_fifo/std_fifo.v work
	vlog std_fifo/std_fifo.v +incdir+./common

$(vsim_asymmetric_fifo) : asymmetric_fifo/asymmetric_fifo.v work $(vsim_asymmetric_distributed_ram)
	vlog asymmetric_fifo/asymmetric_fifo.v +incdir+./common

$(vsim_asymmetric_distributed_ram) : ram/asymmetric_distributed_ram.v work
	vlog ram/asymmetric_distributed_ram.v +incdir+./common

cleanVsim :
	rm -rf work transcript

checkVsim : work $(vsim_pattern_decoder_tb) $(vsim_pattern_decoder) $(vsim_std_fifo) $(vsim_asymmetric_fifo)
	echo -e "vsim work.pattern_decoder_tb\nrun -all" | vsim

vim :
	vim -p pattern_decoder.v pattern_decoder_tb.v xst.mk asymmetric_fifo/asymmetric_fifo.v makefile std_fifo/std_fifo.v

include xst.mk
