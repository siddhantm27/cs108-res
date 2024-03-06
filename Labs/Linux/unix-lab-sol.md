# activity-1
```bash
cp $(find ./input -name "*.*") ./output/
```

# activity-2
```bash
grep "PASSED" result.csv | cut -d ',' -f 1,2
```

# activity-4
```bash
sort -t ':' -r -n -k 2 data.txt > sort.txt
grep "jodhpur" sort.txt > extract.txt
cut -d ':' -f 1,2 --output-delimiter $'\t' extract.txt > display.txt
tar cvzf submission.tar.gz sort.txt extract.txt display.txt
```

# activity-5
```bash
grep -E '^[a-z]*[0-9]{2} [1-4] 22b[0-9]{4}@iitb.ac.in submission.sh$' collect.txt > output.txt
```

