curl -v -i http://www.cs.tufts.edu/comp/112/index.html > tests/o1.txt
curl -x localhost:$1 -v -i http://www.cs.tufts.edu/comp/112/index.html > tests/e1.txt
