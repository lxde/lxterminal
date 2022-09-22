FROM lxterminal-deps-ubuntu

WORKDIR /lxterminal
COPY . .
RUN chmod a+x ./build-install.sh
CMD ["./build-install.sh"]
