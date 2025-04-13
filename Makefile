compose/build:
	docker build --platform=linux/amd64 -t 9cc . 

compose/up: compose/build
	docker run --platform=linux/amd64 -it --rm -v $(CURDIR):/home/user/work 9cc
