SUBS = lib bin

it clean depend install:
	-X=`pwd`; \
        for i in $(SUBS); \
        do \
                echo '<<<' $$i '>>>'; \
                cd $$X/$$i; \
                make $@; \
        done
