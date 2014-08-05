all: clean pylibbvg 

clean:
	python setup.py clean --all
	$(RM) -r pylibbvg.egg-info
	$(RM) -r dist
	$(RM) -r data include src test 

src:  
	mkdir -p src
	cp ../../src/* src
	cp ../bvg.pyx src
	cp ../clibbvg.pxd src

include:
	mkdir -p include
	cp ../../include/* include

test:
	mkdir -p test
	cp ../test/* test

data:
	mkdir -p data
	cp ../../data/* data
	
pylibbvg: src include test data	
	python setup.py build_clib
	python setup.py sdist 
	python setup.py bdist_egg
	python setup.py bdist
	
upload:	
	python setup.py register -r pypi
	python setup.py sdist upload -r pypi	

.PHONY: all clean pylibbvg
