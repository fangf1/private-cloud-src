/***************************************/
/*@Author_Fangwei*/
/*gcc -g -Wall listwf.c -o listout -lvirt*/
/***************************************/

#include <stdlib.h>
#include <stdio.h>
#include <libvirt/libvirt.h>

//#define MAXID 50

/* the data structure of time */
typedef struct timeInfo
{
    long long cpu_time;
    struct timeval real_time;
} timeInfoNode;

/* the hypervisor connection */
static virConnectPtr conn = NULL;

/* release the connect of hypervisor */
void closeConn()
{
    if (conn != NULL)
        virConnectClose(conn);
}

/* release the domain pointer */
void freeDom(virDomainPtr dom)
{
    if (dom != NULL)
        virDomainFree(dom);
}

/* get the start time of each domain */
void getTimeInfo(int id, timeInfoNode * infos)
{
    virDomainPtr dom = NULL;
    virDomainInfo info;
    int ret;

    /* Find the domain of the given id */
    dom = virDomainLookupByID(conn, id);
    if (dom == NULL)
    {
        fprintf(stderr, "Failed to find Domain %d\n", id);
        freeDom(dom);
        closeConn();
    }

    /* Get the information of the domain */
    ret = virDomainGetInfo(dom, &info);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to get information for Domain %d\n", id);
        freeDom(dom);
        closeConn();
    }

    /* get the start of realTime*/
    if (gettimeofday(&(infos->real_time), NULL) ==  - 1)
    {
        fprintf(stderr, "Failed to get start time\n");
			return;
    }

    /* get the start of CPUTime*/
    infos->cpu_time = info.cpuTime; /* nanosecond */

    freeDom(dom);
}

void getDomainInfo(int id, timeInfoNode infos)
{
    virDomainPtr dom = NULL;
    virDomainInfo info;
    int ret;
    struct timeval realTime;
    int cpu_diff, real_diff;
    float usage;

    /* Find the domain of the given id */
    dom = virDomainLookupByID(conn, id);
    if (dom == NULL)
    {
        fprintf(stderr, "Failed to find Domain %d\n", id);
        freeDom(dom);
        closeConn();
    } 

    /* Get the information of the domain */
    ret = virDomainGetInfo(dom, &info);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to get information for Domain %d\n", id);
        freeDom(dom);
        closeConn();
    }

    /* get the end of realTime*/
    if (gettimeofday(&realTime, NULL) ==  - 1)
    {
        fprintf(stderr, "Failed to get start time\n");
        return;
    }

    /* calculate the usage of cpu */
    cpu_diff = (info.cpuTime - infos.cpu_time) / 10000;
    real_diff = 1000 *(realTime.tv_sec - infos.real_time.tv_sec) + 
        (realTime.tv_usec - infos.real_time.tv_usec);
    usage = cpu_diff / (float)(real_diff);

    /* print the results */
    printf("%d\t%.3f%\t%lu\t%lu\t%hu\t%0X\t%s\n", id, usage, info.memory / 1024,
        info.maxMem / 1024, info.nrVirtCpu, info.state, virDomainGetName(dom));

    freeDom(dom);
}

int main()
{
    int idCount;
    int i;
    int id;
    //int ids[MAXID];
	int *ids;
    //timeInfoNode timeInfos[MAXID];

    printf("--------------------------------------------------------\n");
    printf("             XEN Domain Monitor \n");
    printf("--------------------------------------------------------\n");

    /* NULL means connect to local Xen hypervisor */
    conn = virConnectOpenReadOnly(NULL);
    if (conn == NULL)
    {
        fprintf(stderr, "Failed to connect to hypervisor\n");
        closeConn();
        return 0;
    }

	/*char* caps;
	caps = virConnectGetCapabilities(conn);
	printf("Capabilities:\n%s\n",caps);
	free(caps);*/
	char *host;
	host = virConnectGetHostname(conn);
	fprintf(stdout, "Hostname:%s\n",host);
	free(host);
	int vcpus;
	vcpus = virConnectGetMaxVcpus(conn,NULL);
	fprintf(stdout, "Maxmum support vcpus:%d\n",vcpus);
	unsigned long long node_free_memory;
	node_free_memory = virNodeGetFreeMemory(conn);
	fprintf(stdout, "free memory:%lld\n",node_free_memory);
	virNodeInfo nodeinfo;
	virNodeGetInfo(conn,&nodeinfo);
	fprintf(stdout, "Model: %s\n", nodeinfo.model);
    fprintf(stdout, "Memory size: %lukb\n", nodeinfo.memory);
    fprintf(stdout, "Number of CPUs: %u\n", nodeinfo.cpus);
    fprintf(stdout, "MHz of CPUs: %u\n", nodeinfo.mhz);
    fprintf(stdout, "Number of NUMA nodes: %u\n", nodeinfo.nodes);
    fprintf(stdout, "Number of CPU sockets: %u\n", nodeinfo.sockets);
    fprintf(stdout, "Number of CPU cores per socket: %u\n", nodeinfo.cores);
    fprintf(stdout, "Number of CPU threads per core: %u\n", nodeinfo.threads);	
    fprintf(stdout, "Virtualization type: %s\n", virConnectGetType(conn));
	unsigned long ver;
	virConnectGetVersion(conn, &ver);
	fprintf(stdout, "Version: %lu\n", ver);
	/*unsigned long Libver;
	virConnectGetLibVersion(conn, &Libver);
	fprintf(stdout, "Libvirt Version: %lu\n", Libver);*/
	char *uri;
	uri = virConnectGetURI(conn);
	fprintf(stdout, "Canonical URI: %s\n", uri);
	free(uri);
	/* get the count of IDs and save these ID into ids[] */
    idCount = virConnectNumOfDomains(conn);
	ids = malloc(sizeof(int) *idCount);
	idCount = virConnectListDomains(conn,ids,idCount);
	//idCount = virConnectListDomains(conn, &ids[0], MAXID);
    if (idCount < 0)
    {
        fprintf(stderr, "Failed to list the domains\n");
        closeConn();
        return 0;
    }

    timeInfoNode timeInfos[idCount];
	printf("Domain Totals: %d\n", idCount);
    printf("ID\tCPU\tMEM\tMaxMEM\tVCPUs\tState\tNAME\n");

    /* loop get the CPUtime info by IDs */
    for (i = 0; i < idCount; i++)
    {
        id = ids[i];
        getTimeInfo(id, &(timeInfos[i]));
    }

    sleep(1);

    /* loop print the domain info and calculate the usage of cpus*/
    for (i = 0; i < idCount; i++)
    {
        id = ids[i];
        getDomainInfo(id, timeInfos[i]);
    }

    free(ids);
	printf("--------------------------------------------------------\n");
    closeConn();
    return 0;
}
