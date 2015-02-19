FROM flitter/init

RUN apt-get update && apt-get install -y build-essential libssl-dev git-core flex bison pkg-config && \
    adduser --system --home /home/atheme atheme && mkdir /home/atheme/src && \
    chmod 777 /home/atheme/src && cd /home/atheme/src && \
    setuser atheme git clone https://github.com/atheme/atheme && cd atheme && \
    setuser atheme git checkout atheme-7.2.5 && \
    setuser atheme git submodule update --init && \
    setuser atheme wget https://raw.githubusercontent.com/Elemental-IRCd/elemental-ircd/qaohv/extra/services/atheme/elemental-ircd.c -O modules/protocol/elemental-ircd.c &&\
    setuser atheme ./configure && setuser atheme make && setuser atheme make install || true

ADD atheme.conf /home/atheme/atheme/etc/
RUN chmod 777 /home/atheme/atheme/etc/*

ADD run /etc/service/atheme/run

CMD /sbin/my_init
