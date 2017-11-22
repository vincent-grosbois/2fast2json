echo "Running Debug x86:"
time Debug/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_debug.txt
diff result_ref.txt out_x86_debug.txt

echo "Running Debug x86 no avx:"
time Debug/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_debug_no_avx.txt no_avx2
diff result_ref.txt out_x86_debug_no_avx.txt

echo "Running Debug x64:"
time x64/Debug/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_debug.txt
diff result_ref.txt out_x64_debug.txt

echo "Running Debug x64 no avx:"
time x64/Debug/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_debug_no_avx.txt no_avx2
diff result_ref.txt out_x64_debug_no_avx.txt