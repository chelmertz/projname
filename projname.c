#define _GNU_SOURCE
#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * This is my ugly c version of https://github.com/LogIN-/ospnc
 * because it's a fun and limited scope to explore c with (especially limited
 * concerning the nice libcurl).
 *
 * The usage of libcurl hides network stuff but should be interchangable with
 * something more low-levelly (and more performant).
 */

#define CHECKS 6

enum ret {
	NAME_FREE = 0,
	NAME_TAKEN = 1,
	HANDLER_FAILED = 2,
	INTERNAL_ERROR = 3
};

static const char *project;

static char *sites[CHECKS][2] = {
	{ "pypi", "https://pypi.python.org/pypi/%s/" },
	{ "sourceforge", "http://sourceforge.net/projects/%s/" },
	{ "google code", "https://code.google.com/p/%s/"},
	{ "rubygems", "https://rubygems.org/gems/%s" },
	{ "debian", "http://sources.debian.net/src/%s/" },
	{ "launchpad", "https://launchpad.net/%s" },
};

struct search_site {
	char *project_url;
	char *site;
};

/* I don't get it.. why's the * not on the type here, but on the function? */
static struct search_site *search_site_new(const char *site, const char *url_template, const char *project) {
	struct search_site *ss = malloc(sizeof(struct search_site));
	if(!ss) {
		perror("malloc()\n");
		exit(INTERNAL_ERROR);
	}
	if(!asprintf(&ss->site, "%s", site)) {
		perror("strdup()\n");
		exit(INTERNAL_ERROR);
	}
	if(!asprintf(&ss->project_url, url_template, project)) {
		perror("asprintf()\n");
		exit(INTERNAL_ERROR);
	}
	return ss;
};

static void search_site_destroy(struct search_site *ss) {
	if(ss == NULL) {
		return;
	}
	if(ss->project_url != NULL) {
		free(ss->project_url);
	}
	if(ss->site != NULL) {
		free(ss->site);
	}
	free(ss);
}

static size_t curl_devnull(char *ptr, size_t size, size_t nmemb, void *userdata) {
	return size * nmemb;
}

static void *search_by_url(void *url) {
	CURL *curl;
	CURLcode res;
	long *response_code = NULL;
	/* Once again, why is the * on the right hand side here?
	 * I thought it was (struct *search_site) url or so.. */
	struct search_site *ss = (struct search_site*) url;

	if(!url) {
		fprintf(stderr, "search_by_url() need a url\n");
		exit(INTERNAL_ERROR);
	}

	curl = curl_easy_init();

	if(!curl) {
		perror("curl_easy_init()\n");
		exit(HANDLER_FAILED);
	}
	curl_easy_setopt(curl, CURLOPT_URL, ss->project_url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_devnull);
	/* Google code returns CURLE_SSL_CACERT_BADFILE (77). You can tie a
	 * cert to the requests performed if wanted. */
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
	res = curl_easy_perform(curl);
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %d for url %s\n", res, ss->project_url);
		exit(HANDLER_FAILED);
	}
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_getinfo() failed: %d\n", res);
		exit(HANDLER_FAILED);
	}
	if((int) response_code == 200) {
		printf("[%s] taken\n", ss->site);
		curl_easy_cleanup(curl);
		return (void *) NAME_TAKEN;
	} else if((int) response_code == 404) {
		curl_easy_cleanup(curl);
		return (void *) NAME_FREE;
	}
	fprintf(stderr, "What does response code %ld mean for %s?\n", *response_code, ss->site);
	curl_easy_cleanup(curl);
	exit(HANDLER_FAILED);
}

int main(int argc, char *argv[]) {
	void *available = NULL;
	int result = 0;
	struct search_site *ss = NULL;
	struct search_site *cleanup[CHECKS];
	int i = 0;
	int error = 0;
	pthread_t threads[CHECKS];

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <projectname>\n", argv[0]);
		return EXIT_FAILURE;
	}
	project = argv[1];

	curl_global_init(CURL_GLOBAL_ALL);

	for(i = 0; i < CHECKS; i++) {
		ss = search_site_new(sites[i][0], sites[i][1], project);
		cleanup[i] = ss;
		error = pthread_create(&threads[i], NULL, search_by_url, (void *) ss);
		if(error) {
			fprintf(stderr, "pthread_crete() error: %d\n", error);
		}
	}

	for(i = 0; i < CHECKS; i++) {
		error = pthread_join(threads[i], &available);
		if(error) {
			fprintf(stderr, "pthread_join() error: %d\n", error);
		}
		search_site_destroy(cleanup[i]);
		result |= (int) available;
	}

	return result;
}
