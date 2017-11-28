from django.http import HttpResponse

def get_ip(httpRequest):
    # Try getting IP from proxy. If none, then get directly
    try:
        ip_addr = httpRequest.META['HTTP_X_FORWARDED_FOR']
    except KeyError:
        ip_addr = httpRequest.META['REMOTE_ADDR']

    return ip_addr
