curl -v -i http://www.cs.cmu.edu/~dga/dga-headshot.jpg > tests/o3.jpeg
curl -x localhost:$1 -v -i http://www.cs.cmu.edu/~dga/dga-headshot.jpg > tests/e3.jpeg
