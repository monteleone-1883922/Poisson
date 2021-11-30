#./bin/ymodel --scene tests/01_terrain/terrain.json --output outs/01_terrain/terrain.json --terrain object
# ./bin/ymodel --scene tests/02_displacement/displacement.json --output outs/02_displacement/displacement.json --displacement object
# ./bin/ymodel --scene tests/03_hair1/hair1.json --output outs/03_hair1/hair1.json --hairbase object --hair hair
#./bin/ymodel --scene tests/03_hair2/hair2.json --output outs/03_hair2/hair2.json --hairbase object --hair hair --hairlen 0.005 --hairstr 0
# ./bin/ymodel --scene tests/03_hair3/hair3.json --output outs/03_hair3/hair3.json --hairbase object --hair hair --hairlen 0.005 --hairstr 0.01
# ./bin/ymodel --scene tests/03_hair4/hair4.json --output outs/03_hair4/hair4.json --hairbase object --hair hair --hairlen 0.02 --hairstr 0.001 --hairgrav 0.0005 --hairstep 8
# ./bin/ymodel --scene tests/04_grass/grass.json --output outs/04_grass/grass.json --grassbase object --grass grass

#./bin/yscene render outs/01_terrain/terrain.json --output out/01_terrain.jpg --samples 256 --resolution  720
# ./bin/yscene render outs/02_displacement/displacement.json --output out/02_displacement.jpg --samples 256 --resolution  720
# ./bin/yscene render outs/03_hair1/hair1.json --output out/03_hair1.jpg --samples 256 --resolution  720
# ./bin/yscene render outs/03_hair2/hair2.json --output out/03_hair2.jpg --samples 256 --resolution  720
# ./bin/yscene render outs/03_hair3/hair3.json --output out/03_hair3.jpg --samples 256 --resolution  720
# ./bin/yscene render outs/03_hair4/hair4.json --output out/03_hair4.jpg --samples 256 --resolution  720
# ./bin/yscene render outs/04_grass/grass.json --output out/04_grass.jpg --samples 256 --resolution  720 --bounces 128

#./bin/yscene render outs/01_terrain/terrain.json --output out/01_terrain1.jpg --samples 50 --resolution  720 --hairlen 0.01 --hairstr 0.0000001 --hairgrav 0.0005 --hairstep 1

#./bin/yscene render outs/03_hair1/hair1.json --output out/03_hair15.jpg --samples 20 --resolution  720

./bin/ymodel --scene tests/test/test.json --output outs/test/test.json --hairbase object --hair hair --hairlen 0.01 --hairstr 0.00000001 --hairgrav 0.0005 --hairstep 1

./bin/yscene render outs/test/test.json --output out/test2.jpg --samples 20 --resolution  720