.PHONY: all test benchmark

all:
	python3 setup.py build_ext -if
	@echo
	@echo
	@echo UNIVERSAL
	python3 setup.py --hpy-abi=universal build_ext -if

test:
	HPY_ABI=cpython python3 -m pytest tests/tests.py
	HPY_ABI=universal python3 -m pytest tests/tests.py
	HPY_ABI=debug python3 -m pytest tests/tests.py

benchmark:
	python3 -m benchmark.main
