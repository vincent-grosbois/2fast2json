echo ""
echo "Running Release x86 no_training avx2:"
time Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_no_training_avx2_release.txt -1
diff result_ref.txt out_x86_no_training_avx2_release.txt

echo ""
echo "Running Release x86 training1000 avx2:"
time Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_training_1000_avx2_release.txt 1000
diff result_ref.txt out_x86_training_1000_avx2_release.txt

echo ""
echo "Running Release x86 no_training no_avx:"
time Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_release_no_training_no_avx.txt -1 no_avx2
diff result_ref.txt out_x86_release_no_training_no_avx.txt

echo ""
echo "Running Release x86 training1000 no_avx:"
time Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x86_release_training1000_no_avx.txt 1000 no_avx2
diff result_ref.txt out_x86_release_training1000_no_avx.txt

echo ""
echo "Running Release x64 no_training avx2:"
time x64/Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_no_training_avx2_release.txt -1
diff result_ref.txt out_x64_no_training_avx2_release.txt

echo ""
echo "Running Release x64 training1000 avx2:"
time x64/Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_training_1000_avx2_release.txt 1000
diff result_ref.txt out_x64_training_1000_avx2_release.txt

echo ""
echo "Running Release x64 no_training no_avx:"
time x64/Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_release_no_training_no_avx.txt -1 no_avx2
diff result_ref.txt out_x64_release_no_training_no_avx.txt

echo ""
echo "Running Release x64 training1000 no_avx:"
time x64/Release/tftj_main.exe D:\\dev\\2fast2json\\examples\\companies.json name,acquisition.price_amount,image.available_sizes[1],image.available_sizes[1][1],_id out_x64_release_training_1000_no_avx.txt 1000 no_avx2
diff result_ref.txt out_x64_release_training_1000_no_avx.txt