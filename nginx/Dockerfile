FROM nginx:latest

COPY default.conf /etc/nginx/conf.d/default.conf

COPY polla.html /usr/share/nginx/html/

EXPOSE 443

CMD ["nginx", "-g", "daemon off;"]
