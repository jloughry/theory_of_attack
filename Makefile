target = theory_of_attack

latex_cmd = pdflatex

all:: $(target).pdf

$(target).pdf: $(target).tex Makefile
	$(latex_cmd) $(target).tex
	bibtex $(target)
	@if (grep "Warning" $(target).blg > /dev/null ) then false; fi
	@while grep -E "There were undefined references|Rerun to get" \
		$(target).log ; do \
			$(latex_cmd) $(target) ; \
	done

vi:
	vi $(target).tex

touch:
	touch $(target).tex

clean::
	@echo "This is \"clean\" in the local Makefile."
	rm -fv *.log *.blg *.aux *.bbl $(target).out

spell::
	detex $(target).tex | aspell --lang=EN_GB list | sort --ignore-case | uniq -i

allclean: clean
	rm -fv $(target).pdf

include common.mk
