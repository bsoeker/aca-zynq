Step-by-Step to understand the workflow:
1 - git clone https://github.com/LukeVassallo/hd-aca-25.git
2 - Open vivado cd into this cloned repo and execute "source scripts/...."
3 - Follow the pdf instructions also disable the insruction and data caches of the microblaze
so you end up with only one slave port
4 - Generate Block design and export the .xsa for Vitis
5 - Don't forget to generate the bitstream and program the fpga board before running your programs on
Vitis. Otherwise the fpga won't have the MicroBlaze softcore processor and won't run your code!!!

