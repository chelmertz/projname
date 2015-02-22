#define _GNU_SOURCE
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

/* TODO s/get/head ? */

enum ret {
	NAME_FREE = 0,
	NAME_TAKEN = 1,
	HANDLER_FAILED = 2
};

static size_t curl_devnull(char *ptr, size_t size, size_t nmemb, void *userdata) {
	return size * nmemb;
}

static int request_google_code(char *project) {
	CURL *curl;
	CURLcode res;
	long *response_code;
	char *url;

	if(!asprintf(&url, "https://code.google.com/p/%s/", project)) {
		perror("Could not asprintf()\n");
		return HANDLER_FAILED;
	}

	curl = curl_easy_init();
	if(!curl) {
		perror("Curl could not init\n");
		return HANDLER_FAILED;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_devnull);
	res = curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, response_code);
	free(url);
	if(res != CURLE_OK) {
		perror("Curl went wrong\n");
		return HANDLER_FAILED;
	}
	curl_easy_cleanup(curl);
	if(*response_code == 200) {
		printf("[google code] taken\n");
		return NAME_TAKEN;
	} else if(*response_code == 404) {
		return NAME_FREE;
	}
	fprintf(stderr, "What does response code %ld mean for google code?\n", *response_code);
	return HANDLER_FAILED;
}

static int request_sourceforge(char *project) {
	CURL *curl;
	CURLcode res;
	long *response_code;
	char *url;

	if(!asprintf(&url, "http://sourceforge.net/projects/%s/", project)) {
		perror("Could not asprintf()\n");
		return HANDLER_FAILED;
	}

	curl = curl_easy_init();
	if(!curl) {
		perror("Curl could not init\n");
		return HANDLER_FAILED;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_devnull);
	res = curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, response_code);
	free(url);
	if(res != CURLE_OK) {
		perror("Curl went wrong\n");
		return HANDLER_FAILED;
	}
	curl_easy_cleanup(curl);
	if(*response_code == 200) {
		printf("[sourceforge] taken\n");
		return NAME_TAKEN;
	} else if(*response_code == 404) {
		return NAME_FREE;
	}
	fprintf(stderr, "What does response code %ld mean for sourceforge?\n", *response_code);
	return HANDLER_FAILED;
}

static int request_pypi(char *project) {
	CURL *curl;
	CURLcode res;
	long *response_code;
	char *url;

	if(!asprintf(&url, "https://pypi.python.org/pypi/%s/", project)) {
		perror("Could not asprintf()\n");
		return HANDLER_FAILED;
	}

	curl = curl_easy_init();
	if(!curl) {
		perror("Curl could not init\n");
		return HANDLER_FAILED;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_devnull);
	res = curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, response_code);
	free(url);
	if(res != CURLE_OK) {
		perror("Curl went wrong\n");
		return HANDLER_FAILED;
	}
	curl_easy_cleanup(curl);
	if(*response_code == 200) {
		printf("[pypi] taken\n");
		return NAME_TAKEN;
	} else if(*response_code == 404) {
		return NAME_FREE;
	}
	fprintf(stderr, "What does response code %ld mean for pypi?\n", *response_code);
	return HANDLER_FAILED;
}



int main(int argc, char *argv[]) {
	char *project;
	int free = 0;
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <projectname>\n", argv[0]);
		return EXIT_FAILURE;
	}
	project = argv[1];
	free = free | request_google_code(project);
	free = free | request_sourceforge(project);
	return free;
}
