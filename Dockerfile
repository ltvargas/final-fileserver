#FROM gcc:9.4.0-buster

FROM alpine as build-env
RUN apk add --no-cache build-base
WORKDIR /FileServer
COPY . .
# Compile the binaries
RUN make
FROM alpine
EXPOSE 8000
COPY --from=build-env /FileServer/s_server /FileServer/s_server
WORKDIR /s_server
CMD ["/FileServer/s_server","8000"] 
