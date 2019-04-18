curl -v -i http://www.cs.tufts.edu/comp/112/index.html > e1.txt 2>&1
curl -v -i -x localhost:$1 http://www.cs.tufts.edu/comp/112/index.html > o1.txt 2>&1

colordiff e1.txt o1.txt
rm e1.txt
rm o1.txt
