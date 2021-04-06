CC=sdcc
IHX2SMS=ihx2sms
DEVKITSMS=/root/devkitSMS
VERSION_MAJ=1
VERSION_MIN=2
VERSION_STR=\"$(VERSION_MAJ).$(VERSION_MIN)\"
CFLAGS=-mz80 -I$(DEVKITSMS)/SMSlib/src/ --peep-file $(DEVKITSMS)/SMSlib/src/peep-rules.txt -I$(DEVKITSMS)/PSGlib/src/ -DVERSION_STR=$(VERSION_STR) -DVERSION_MAJ=$(VERSION_MAJ) -DVERSION_MIN=$(VERSION_MIN)
LDFLAGS=--no-std-crt0 --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 $(DEVKITSMS)/crt0/crt0_sms.rel $(DEVKITSMS)/PSGlib/PSGlib.rel
#LDFLAGS=--no-std-crt0 --data-loc 0xC000 -Wl-b_BANK2=0x8000 $(DEVKITSMS)/crt0/crt0_sms.rel $(DEVKITSMS)/PSGlib/PSGlib.rel
#LDFLAGS=--no-std-crt0 --data-loc 0xC000 $(DEVKITSMS)/crt0/crt0_sms.rel $(DEVKITSMS)/PSGlib/PSGlib.rel

BANK2_OBJS=data_bank2.rel
BANK3_OBJS=data_bank3.rel

OBJS=main.rel data.rel util.rel savedata.rel flash.rel savestruct.rel inlib.rel vfont.rel knob.rel sinlut.rel

# Due to ihx2sms limitations, the order of ROM banks is important! (see documentation)
ALL_OBJS=$(OBJS) $(BANK2_OBJS) $(BANK3_OBJS)

ROM=sms_etch.sms

all: $(ROM)

flash:
	../../../smscprog1/client/dumpcart.py -p $(ROM)


main.rel: main.c
	$(CC) $(CFLAGS) -c $<

flash.rel: flash.c
	$(CC) $(CFLAGS)  -c $<

data_bank2.rel: data_bank2.c
	$(CC) $(CFLAGS) --codeseg BANK2 --constseg BANK2 -c $<

data_bank3.rel: data_bank3.c
	$(CC) $(CFLAGS) --codeseg BANK3 --constseg BANK3 -c $<

%.rel: %.c
	$(CC) $(CFLAGS) -c $<

%.rel: %.c %.h
	$(CC) $(CFLAGS) -c $<

sms_etch.ihx:	$(ALL_OBJS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^ -lSMSlib

$(ROM): sms_etch.ihx
	$(IHX2SMS) $< $@

clean:
	rm $(OBJS) *.ihx *.sms

run:
	./m.sh
	/usr/src/raph/meka/meka/meka $(ROM)

run_emulicious:
	java -jar /usr/src/raph/emulicious/Emulicious.jar ./$(ROM)
