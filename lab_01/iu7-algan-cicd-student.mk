ready/report.pdf: report/report.pdf
	mkdir -p ./ready
	cp report/report.pdf ready/report.pdf

ready/stud-unit-test-report-prev.json: code/stud-unit-test-report-prev.json
	mkdir -p ./ready
	cp code/stud-unit-test-report-prev.json ready/stud-unit-test-report-prev.json

ready/stud-unit-test-report.json: code/stud-unit-test-report.json
	mkdir -p ./ready
	cp code/stud-unit-test-report.json ready/

ready/app-cli-debug:
	mkdir -p ./ready
	cd code && make
	cp code/app.exe ready/app-cli-debug

.PHONY: clean
clean:
	cd code && make clean
	echo OK