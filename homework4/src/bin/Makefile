SUBS = my_fortune install_data monitor_shm install_and_monitor

it clean depend install:
	-X=`pwd`; \
        for i in $(SUBS); \
        do \
                echo '<<<' $$i '>>>'; \
                cd $$X/$$i; \
                make $@; \
        done
		