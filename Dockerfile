FROM deis/init
MAINTAINER Xena <xena@yolo-swag.com>

# Update base system
RUN apt-get update
RUN apt-get upgrade -yq # 08-02-14

RUN apt-get -yq install build-essential autoconf libssl-dev
RUN apt-get -yq install flex bison

RUN adduser --system --home /home/ircd ircd
RUN mkdir /home/ircd/src
RUN chmod 777 /home/ircd/src

ADD . /home/ircd/src
ADD extra/runit/ /etc/service/ircd/

RUN cd /home/ircd/src; ./configure --prefix=/home/ircd/run ; make ; make install
ADD doc/example.conf /home/ircd//run/etc/ircd.conf

RUN chmod -R 777 /home/ircd/run

EXPOSE 6667

ENTRYPOINT /sbin/my_init
