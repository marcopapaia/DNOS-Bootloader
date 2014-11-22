echo dissassembling entry.bin ...
ndisasm entry/entry.bin -k 0x193,100
ndisasm entry/entry.bin -b 32 -e 0x193

echo dissassembling pmode_text.bin ...
ndisasm -b 32 pmode_text.bin

