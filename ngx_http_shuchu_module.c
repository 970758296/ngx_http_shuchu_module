#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
	ngx_str_t content;
} ngx_http_shuchu_loc_conf_t;


static char *
ngx_http_shuchu(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void *
ngx_http_shuchu_create_loc_conf(ngx_conf_t *cf);

static char *
ngx_http_shuchu_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t
ngx_http_shuchu_handler(ngx_http_request_t *r);

static ngx_int_t
ngx_http_shuchu_init(ngx_conf_t *cf);

static ngx_command_t ngx_http_shuchu_commands[] = {
	{
		ngx_string("shuchu"),
		NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
		ngx_http_shuchu,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_shuchu_loc_conf_t, content),
		NULL,
	},
	ngx_null_command,
};

static ngx_http_module_t  ngx_http_shuchu_ctx = {
	NULL,
	ngx_http_shuchu_init,
	NULL,
	NULL,
	NULL,
	NULL,
	ngx_http_shuchu_create_loc_conf,
	ngx_http_shuchu_merge_loc_conf
};

ngx_module_t ngx_http_shuchu_module = {
	NGX_MODULE_V1,
	&ngx_http_shuchu_ctx,        /* module context */
	ngx_http_shuchu_commands,    /* module directives */
	NGX_HTTP_MODULE,                     /* module type */
	NULL,                                /* init master */
	NULL,                                /* init module */
	NULL,                                /* init process */
	NULL,                                /* init thread */
	NULL,                                /* exit thread */
	NULL,                                /* exit process */
	NULL,                                /* exit master */
	NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_shuchu_init(ngx_conf_t *cf)
{
		ngx_http_handler_pt        *h;
        ngx_http_core_main_conf_t  *cmcf;

        cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

        h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
        if (h == NULL) {
            return NGX_ERROR;
        }

        *h = ngx_http_shuchu_handler;

        return NGX_OK;
}

static char *
ngx_http_shuchu(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	//ngx_http_core_loc_conf_t *clcf;
	//clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	//clcf->handler = ngx_http_shuchu_handler;
	ngx_conf_set_str_slot(cf,cmd,conf);
	return NGX_CONF_OK;
}

static void *
ngx_http_shuchu_create_loc_conf(ngx_conf_t *cf)
{
		ngx_http_shuchu_loc_conf_t *conf;
		conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_shuchu_loc_conf_t));
		if (conf == NULL) {
			return NULL;
		}
		conf->content.len=0;
		conf->content.data = NULL;
		return conf;
}


static char *
ngx_http_shuchu_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
		ngx_http_shuchu_loc_conf_t *prev = parent;
		ngx_http_shuchu_loc_conf_t *conf = child;
		ngx_conf_merge_str_value(conf->content, prev->content, '"');
		return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_shuchu_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_shuchu_loc_conf_t *slcf;
	slcf = ngx_http_get_module_loc_conf(r,ngx_http_shuchu_module);
	if(!(r->method & (NGX_HTTP_HEAD|NGX_HTTP_GET|NGX_HTTP_POST)))
	{
		return NGX_HTTP_NOT_ALLOWED;
	}

	r->headers_out.content_type.len= sizeof("text/html") - 1;
	r->headers_out.content_type.data = (u_char *) "text/html";
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = slcf->content.len;
	if(r->method == NGX_HTTP_HEAD)
	{
		rc = ngx_http_send_header(r);
		if(rc != NGX_OK)
		{
			return rc;
		}
	}
	b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
	if(b == NULL)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
	out.buf = b;
	out.next = NULL;
	b->pos = slcf->content.data;
	b->last = slcf->content.data + (slcf->content.len);
	b->memory = 1;
	b->last_buf = 1;
	rc = ngx_http_send_header(r);
	if(rc != NGX_OK)
	{
		return rc;
	}
	return ngx_http_output_filter(r, &out);
}
