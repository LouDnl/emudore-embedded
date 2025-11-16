# Excerpt from psid64 finale makefile

define my_xa_recipe
#if xa -v -XMASM -c -R -G
#if xa -v -CA65 -c -R -G
if xa -v -l psiddrv-labels -XMASM -c -R $< -o - $(1) | \
od -v -An -tx1 | \
sed -re 's/[[:alnum:]]+/0x&,/g' -e 's/[[:space:]]+//g' -e '/^$$/d' > $@ ; \
then true ; \
else status=$$? ; $(RM) $@ ; exit $$status ; \
fi
endef

.h : .a65
	$(call my_xa_recipe,)
psiddrv.h: psiddrv.a65
	$(call my_xa_recipe,)
psidextdrv.h: psiddrv.a65
	$(call my_xa_recipe,-DSCREEN -DSMOOTH_SCROLL)

clean:
	$(RM) psiddrv.h psidextdrv.h

all: clean psiddrv.h psidextdrv.h

#set -o pipefail ;
