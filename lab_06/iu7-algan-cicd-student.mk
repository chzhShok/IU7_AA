report/report.pdf: report/report.tex
	mkdir -p report
	cd report && pdflatex -interaction=nonstopmode report.tex && pdflatex -interaction=nonstopmode report.tex && pdflatex -interaction=nonstopmode report.tex

ready/report.pdf: report/report.pdf
	mkdir -p ./ready
	chmod +x compress.sh
	./compress.sh report/ 
	cp report/report.pdf ready/report.pdf

ready/stud-unit-test-report-prev.json: code/stud-unit-test-report-prev.json
	mkdir -p ./ready
	cp code/stud-unit-test-report-prev.json ready/stud-unit-test-report-prev.json

ready/stud-unit-test-report.json: code/stud-unit-test-report.json
	mkdir -p ./ready
	cp code/stud-unit-test-report.json ready/

ready/app-cli-debug:
	mkdir -p ./ready
	printf '#!/bin/sh\npython3 -m code.main "$$@"\n' > ready/app-cli-debug
	chmod +x ready/app-cli-debug

ready/app-cli-release: ready/app-cli-debug
	cp ready/app-cli-debug ready/app-cli-release

.PHONY: clean
clean:
	rm -rf ready
	find . -name '__pycache__' -type d -prune -exec rm -rf {} +
	rm -f report/report.pdf
	echo OK
