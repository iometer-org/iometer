# Generates online and print versions of XML source file.

BASENAME=iom

# SGML source file.
XML_FILE=$(BASENAME).xml

# Stylesheets - EDIT FOR YOUR LOCAL CONFIG

XSL_HTML_MULTI=/usr/local/share/sgml/xsl/html/tldp-html-chunk.xsl
XSL_HTML=/usr/local/share/sgml/xsl/html/tldp-html.xsl
XSL_FO=/usr/local/share/sgml/xsl/fo/tldp-print.xsl

# Generated files.
HTML_FILE=index.html
HTML_SING_FILE=$(BASENAME).html
FO_FILE=$(BASENAME).fo

# Build rules.

html: $(HTML_FILE)

singlehtml: $(HTML_SING_FILE)

fo: $(FO_FILE)

valid: 
	xmllint --valid --noout $(XML_FILE)

copy:
	mount /dev/fd0
	rm -r /mnt/floppy/*
	cp *.html /mnt/floppy
	cp *.fo /mnt/floppy
	cp *.css /mnt/floppy
	mkdir /mnt/floppy/images
	cp images/* /mnt/floppy/images
	umount /dev/fd0

copylocal:
	cp *.html /usr/local/apache2/htdocs/iom
	cp images/* /usr/local/apache2/htdocs/iom/images
	cp *.css /usr/local/apache2/htdocs/iom

pdf:
	pdflatex "&pdfxmltex" iom.fo
	pdflatex "&pdfxmltex" iom.fo

all: valid html singlehtml fo

clean:
	rm -f *.html
	rm -f *.fo
	rm -f *.pdf
	rm -f *.aux
	rm -f *.log
	rm -f *.out

# Compile rules.

$(HTML_FILE): $(XML_FILE)
	xsltproc \
		--stringparam use.role.for.mediaobject 1 \
		$(XSL_HTML_MULTI) \
		$(XML_FILE)

$(HTML_SING_FILE): $(XML_FILE)
	xsltproc \
		--output $(BASENAME).html \
		--stringparam use.role.for.mediaobject 1 \
		$(XSL_HTML) \
		$(XML_FILE)
	
$(FO_FILE): $(XML_FILE)
	xsltproc --output $(BASENAME).fo \
		--stringparam use.role.for.mediaobject 1 \
		--stringparam fop.extensions 1 \
		$(XSL_FO) \
		$(XML_FILE)

