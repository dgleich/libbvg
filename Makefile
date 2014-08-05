.PHONY: push docs

push:
	git push origin gh-pages
    
docs:
	cd .. && $(MAKE) docs
	
