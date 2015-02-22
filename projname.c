#define _GNU_SOURCE
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO s/get/head ? */
/* TODO parallelize calls */

enum ret {
	NAME_FREE = 0,
	NAME_TAKEN = 1,
	HANDLER_FAILED = 2,
	INTERNAL_ERROR = 4
};

static size_t curl_devnull(char *ptr, size_t size, size_t nmemb, void *userdata) {
	return size * nmemb;
}

static int search_by_url(const char *site, const char *url_template, const char *project) {
	CURL *curl;
	CURLcode res;
	long *response_code = NULL;
	char *url = NULL;

	if(!asprintf(&url, url_template, project)) {
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
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %d\n", res);
		return HANDLER_FAILED;
	}
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_getinfo() failed: %d\n", res);
		return HANDLER_FAILED;
	}
	free(url);
	curl_easy_cleanup(curl);
	if((int) response_code == 200) {
		printf("[%s] taken\n", site);
		return NAME_TAKEN;
	} else if((int) response_code == 404) {
		return NAME_FREE;
	}
	fprintf(stderr, "What does response code %ld mean for %s?\n", *response_code, site);
	return HANDLER_FAILED;
}

int main(int argc, char *argv[]) {
	char *project;
	int available = 0;
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <projectname>\n", argv[0]);
		return EXIT_FAILURE;
	}
	project = argv[1];

	available = available | search_by_url("pypi", "https://pypi.python.org/pypi/%s/", project);
	available = available | search_by_url("sourceforge", "http://sourceforge.net/projects/%s/", project);
	available = available | search_by_url("google code", "https://code.google.com/p/%s/", project);

	return available;
}
