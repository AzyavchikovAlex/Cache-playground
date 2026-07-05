 seq -f "%03g" 1 20 | xargs -I {} -P 4 python3 ./parser.py --input=./cluster{}.txt --output=../cluster{}-parsed.txt
