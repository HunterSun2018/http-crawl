http-crawl : src/main.cpp src/http_client.cpp
	g++	src/main.cpp src/http_client.cpp \
	-I ~/boost_1_74_0 \
	-DBOOST_ASIO_HAS_CO_AWAIT -DBOOST_ASIO_HAS_STD_COROUTINE \
	-fcoroutines \
	-lssl -lcrypto -lpthread \
	-o http-crawl


clean:
	rm http-crawl