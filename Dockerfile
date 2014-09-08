FROM flitter/init
MAINTAINER Xena <xena@yolo-swag.com>

# Update base system
RUN apt-get update && apt-get upgrade -yq && \
    apt-get -yq install build-essential autoconf libssl-dev flex bison && \
    adduser --system --home /home/ircd ircd && \
    mkdir /home/ircd/src && \
    chmod 777 /home/ircd/src

ADD . /home/ircd/src

RUN cd /home/ircd/src; ./configure --prefix=/home/ircd/run ; make ; make install

ADD doc/example.conf /home/ircd/run/etc/ircd.conf
ADD extra/runit/ /etc/service/ircd/

RUN chmod -R 777 /home/ircd/run

EXPOSE 6667

ENTRYPOINT /sbin/my_init
