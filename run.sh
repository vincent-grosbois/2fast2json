echo "Running Release x86:"
time Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_release.txt
diff result_ref.txt out_x86_release.txt

echo "Running Release x86 no avx:"
time Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_release_no_avx.txt no_avx2
diff result_ref.txt out_x86_release_no_avx.txt

echo "Running Release x64:"
time x64/Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_release.txt
diff result_ref.txt out_x64_release.txt

echo "Running Release x64 no avx:"
time x64/Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_release_no_avx.txt no_avx2
diff result_ref.txt out_x64_release_no_avx.txt