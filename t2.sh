curl -v -i http://www.cs.cmu.edu/ > e2.txt 2>&1
curl -v -i -x localhost:$1 http://www.cs.cmu.edu/ > o2.txt 2>&1

colordiff e2.txt o2.txt
rm e2.txt
rm o2.txt