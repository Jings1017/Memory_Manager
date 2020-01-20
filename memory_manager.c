#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node
{
    int pfi;
    int reference;
    struct node* next;
    struct node* pre;
};

int main(int argc, char *argv[])
{
    char line[150000][30];
    long long int line_count=0;
    while(fgets(line[line_count],sizeof(line[line_count]),stdin)!=NULL)
    {
        line_count++;
    }
    char policy[10];
    sscanf(line[0],"Policy: %s",policy);

    char VP[10];
    sscanf(line[1],"Number of Virtual Page: %s",VP);
    int virtual_page = atoi(VP);

    char PF[10];
    sscanf(line[2],"Number of Physical Frame: %s",PF);
    int physical_frame = atoi(PF);

    long long int H=0,M=0;
    double miss_rate;

    if(strcmp(policy,"FIFO")==0)
    {
        int PFI[physical_frame],index=0;
        int Source,Destination,Evicted;
        int last_PFI[physical_frame];
        int disk[virtual_page];
        // initial
        for(long long int i=0; i<physical_frame; i++)
        {
            PFI[i]=-1;
        }
        for(int i=0; i<virtual_page; i++)
        {
            last_PFI[i] = -1;
            disk[i] = -1;
        }
        int full=0;
        for(int i=4 ; i<line_count; i++)
        {
            int target=-1;
            if(strncmp(line[i],"Write",5)==0)
            {
                char w[5] ;
                sscanf(line[i],"Write %s",w);
                target = atoi(w);
            }
            else if(strncmp(line[i],"Read",4)==0)
            {
                char r[5] ;
                sscanf(line[i],"Read %s",r);
                target = atoi(r);
            }

            // first come
            if(full<physical_frame)
            {
                int hit=0;
                for(int h=0; h<physical_frame; h++)
                {
                    if(PFI[h]==target)
                    {
                        printf("Hit, %d=>%d\n",target,h);
                        H++;
                        hit=1;
                    }
                }

                if(hit==0)
                {
                    PFI[full] = target;
                    printf("Miss, %d, -1>>-1, %d<<-1\n",full,target);
                    last_PFI[target]=full;
                    M++;
                    full++;
                }
            }

            if(full==physical_frame)
            {
                full++;
                continue;
            }

            if(full>physical_frame)
            {
                for(int k=0; k<physical_frame; k++)
                {
                    // if target is in PFI now
                    int check=0;
                    for(int j=0; j<physical_frame; j++)
                    {
                        if(target==PFI[j])
                            check++;
                    }

                    // update the value
                    if(k==index && check==0 && PFI[k]!=-1)
                    {
                        Evicted = PFI[k];
                        PFI[k] = target;
                        //Destination = k;
                        Source = last_PFI[target];
                        last_PFI[target] = k;
                        // put evicted in disk
                        for(int s=0; s<virtual_page; s++)
                        {
                            if(disk[s]==-1)
                            {
                                disk[s] = Evicted;
                                Destination = s;
                                break;
                            }
                        }
                        // find target in disk
                        for(int a=0; a<virtual_page; a++)
                        {
                            if(disk[a]==target)
                            {
                                disk[a] = -1;
                                Source = a;
                                break;
                            }
                        }
                        printf("Miss, %d, %d>>%d, %d<<%d\n",k,Evicted,Destination,target,Source);
                        index++;
                        index %= physical_frame;
                        M++;
                    }
                    // hit
                    else if(target==PFI[k] && check==1)
                    {
                        printf("Hit, %d=>%d\n",target,k);
                        H++;
                    }
                }
            }
        }
        miss_rate = M;
        miss_rate /= (M+H);
        printf("Page Fault Rate: %.3f\n",miss_rate);
    }
    else if(strcmp(policy,"ESCA")==0)
    {
        int PFI[physical_frame];
        int dirty[physical_frame];
        int reference[physical_frame];
        int disk[virtual_page];
        int ptr=-1, temp=-1;

        // init
        for(int i=0; i<virtual_page; i++)
        {
            disk[i] = -1;
        }
        for(int i=0; i<physical_frame; i++)
        {
            PFI[i] = -1;
            dirty[i] = 0;
            reference[i] = 0;
        }

        for(long long int i=4; i<line_count; i++)
        {
            int rw =-1;
            int target = -1;
            int exist = 0;
            if(strncmp(line[i],"Write",5)==0)
            {
                char w[5] ;
                sscanf(line[i],"Write %s",w);
                target = atoi(w);
                rw = 1;
            }
            else if(strncmp(line[i],"Read",4)==0)
            {
                char r[5] ;
                sscanf(line[i],"Read %s",r);
                target = atoi(r);
                rw = 0;
            }

            for(int j=0; j<physical_frame; j++)
            {
                // Hit
                if(PFI[j] == target)
                {
                    H++;
                    exist = 1;
                    // write
                    if(rw == 1 || (reference[j]==1 && dirty[j]==1))
                    {
                        reference[j] =1;
                        dirty[j] =1;
                    }
                    // 01  read
                    else if((reference[j]==0 && dirty[j]==1) && rw==0)
                    {
                        reference[j] =1;
                        dirty[j] =1;
                    }
                    else
                    {
                        reference[j] =1;
                        dirty[j] =0;
                    }
                    printf("Hit, %d=>%d\n",target,j);
                }
            }

            if(exist==0)
            {
                M++;
                int evicted = -1, done=0, notfull=-1, destination=-1, source=-1;

                for(int j=0; j<physical_frame; j++)
                {
                    if(PFI[j]==-1)
                    {
                        notfull = j;
                        break;
                    }
                }

                // not full
                if(notfull != -1)
                {
                    PFI[notfull] = target;
                    if(rw == 1)
                    {
                        reference[notfull] = 1;
                        dirty[notfull] = 1;
                    }
                    else
                    {
                        reference[notfull] = 1;
                        dirty[notfull] = 0;
                    }
                }
                // full
                else
                {
                    // find 00
                    for(int j=1 ; j<=physical_frame ; j++)
                    {
                        temp = (ptr+j)%physical_frame;
                        // 00
                        if(reference[temp]==0 && dirty[temp]==0)
                        {
                            evicted = PFI[temp];
                            PFI[temp] = target;
                            // write
                            if(rw==1)
                            {
                                reference[temp] = 1;
                                dirty[temp] = 1;
                            }
                            // read
                            else
                            {
                                reference[temp] = 1;
                                dirty[temp] = 0;
                            }
                            ptr = temp;
                            done =1 ;
                            break;
                        }
                    }
                    if(done==0)
                    {
                        // find 01
                        for(int j=1 ; j<=physical_frame ; j++)
                        {
                            temp = (ptr+j)%physical_frame;
                            // 01
                            if(reference[temp]==0 && dirty[temp]==1)
                            {
                                evicted = PFI[temp];
                                PFI[temp] = target;
                                if(rw == 1)
                                {
                                    reference[temp] = 1;
                                    dirty[temp] = 1;
                                }
                                else
                                {
                                    reference[temp] = 1;
                                    dirty[temp] = 0;
                                }
                                ptr = temp ;
                                done = 1;
                                break;
                            }
                            // clear ref bit
                            else
                            {
                                if(reference[temp]==1 && dirty[temp]==0)
                                {
                                    reference[temp] = 0;
                                    dirty[temp] = 0;
                                }
                                else if(reference[temp]==1 && dirty[temp]==1)
                                {
                                    reference[temp] = 0;
                                    dirty[temp] = 1;
                                }
                            }
                        }
                    }
                    if(done==0)
                    {
                        // find 00
                        for(int j=1 ; j<=physical_frame ; j++)
                        {
                            temp = (ptr+j)%physical_frame;
                            // 00
                            if(reference[temp]==0 && dirty[temp]==0)
                            {
                                evicted = PFI[temp];
                                PFI[temp] = target;
                                // write
                                if(rw == 1)
                                {
                                    reference[temp] =1;
                                    dirty[temp] =1;
                                }
                                // read
                                else
                                {
                                    reference[temp] = 1;
                                    dirty[temp] = 0;
                                }
                                ptr = temp ;
                                done = 1;
                                break;
                            }
                        }
                    }
                    if(done==0)
                    {
                        // fine 01
                        for(int j=1 ; j<=physical_frame ; j++)
                        {
                            temp = (ptr+j)%physical_frame;
                            // 01
                            if(reference[temp]==0 && dirty[temp]==1)
                            {
                                evicted = PFI[temp];
                                PFI[temp] = target;
                                if(rw == 1)
                                {
                                    reference[temp] =1;
                                    dirty[temp] =1;
                                }
                                else
                                {
                                    reference[temp] =1;
                                    dirty[temp] =0;
                                }
                                ptr = temp;
                                break;
                            }
                        }
                    }
                    // put in disk
                    if(evicted != -1)
                    {
                        for(int j=0 ; j<virtual_page ; j++)
                        {
                            if(disk[j] == -1)
                            {
                                disk[j] = evicted;
                                destination = j;
                                break;
                            }
                        }
                    }
                }
                for(int k=0; k<virtual_page; k++)
                {
                    if(disk[k]==target)
                    {
                        source = k;
                        disk[k] = -1;
                    }
                }

                if(notfull != -1)
                    printf("Miss, %d, %d>>%d, %d<<%d\n",notfull,evicted,destination,target,source);
                else
                    printf("Miss, %d, %d>>%d, %d<<%d\n",ptr,evicted,destination,target,source);
            }
        }
        miss_rate = M;
        miss_rate /= (M+H);
        printf("Page Fault Rate: %.3f\n",miss_rate);
    }
    else if(strcmp(policy,"SLRU")==0)
    {
        int PFI[physical_frame];
        int list_type[physical_frame];
        int disk[virtual_page];
        int frame_in_active=0, frame_in_inactive=0;
        int inactive_num = physical_frame/2;
        int active_num = physical_frame/2;
        if(physical_frame%2==1)
            inactive_num +=1;

        for(int i=0; i<virtual_page; i++)
        {
            disk[i] = -1;
        }
        for(int i=0; i<physical_frame; i++)
        {
            // PFI
            PFI[i] = -1;
            // active bit
            list_type[i] = 0;
        }

        struct node *active_head = (struct node*)malloc(sizeof(struct node*));
        struct node *active_tail = active_head;
        struct node *inactive_head = (struct node*)malloc(sizeof(struct node*));
        struct node *inactive_tail = (struct node*)malloc(sizeof(struct node*));
        struct node *remove = (struct node*)malloc(sizeof(struct node*));
        struct node *temp = (struct node*)malloc(sizeof(struct node*));

        int ptr=0;
        for(int i=4; i<line_count; i++)
        {
            int target=-1;
            if(strncmp(line[i],"Write",5)==0)
            {
                char w[5] ;
                sscanf(line[i],"Write %s",w);
                target = atoi(w);
            }
            else if(strncmp(line[i],"Read",4)==0)
            {
                char r[5] ;
                sscanf(line[i],"Read %s",r);
                target = atoi(r);
            }

            int exist = 0, list, space = physical_frame;
            int evicted =-1, Destination=-1, Source =-1;

            // Hit
            for(int j=0; j<physical_frame; j++)
            {
                if(PFI[j]==target)
                {
                    exist=1;
                    list = list_type[j];
                    ptr = j;
                    printf("Hit, %d=>%d\n",target,ptr);
                }
            }

            // in list
            if(exist)
            {
                H++;
                // in active list
                if(list)
                {
                    temp = (struct node*)malloc(sizeof(struct node*));
                    temp = active_head;

                    while(temp)
                    {
                        if(temp->pfi == ptr)
                            break;
                        temp = temp->next;
                    }

                    temp->reference = 1;

                    // delete from active list
                    if(temp->next!=NULL && temp->pre!=NULL)
                    {
                        temp->pre->next = temp->next;
                        temp->next->pre = temp->pre;

                        temp->next = active_head;
                        active_head->pre = temp;

                        temp->pre = NULL;
                        active_head = temp;
                    }
                    else if(temp->pre != NULL)
                    {
                        temp->pre->next = NULL;
                        active_tail = active_tail->pre;

                        temp->next = active_head;
                        active_head->pre = temp;
                        temp->pre = NULL;
                        active_head = temp;
                    }
                    else
                    {
                        temp->reference = 1;
                    }
                }
                // in inactive list
                else
                {
                    temp = (struct node*)malloc(sizeof(struct node*));
                    temp = inactive_head;

                    while(temp != NULL)
                    {
                        if(temp->pfi == ptr)
                            break;
                        temp = temp->next;
                    }

                    // reference = 1
                    // clear ref bit and move the page to active list head
                    if(temp->reference == 1)
                    {
                        if(temp->next != NULL && temp->pre != NULL)
                        {
                            temp->pre->next = temp->next;
                            temp->next->pre = temp->pre;
                        }
                        else if(temp->next!=NULL)
                        {
                            temp->next->pre = NULL;
                            inactive_head = inactive_head->next;
                        }
                        else if(temp->pre!=NULL)
                        {
                            temp->pre->next = NULL;
                            inactive_tail = inactive_tail->pre;
                        }

                        // add to active list
                        // active list is empty
                        if(frame_in_active == 0)
                        {
                            temp->next = NULL;
                            temp->pre = NULL;
                            temp->reference = 0;
                            active_head = temp;
                            active_tail = temp;
                            list_type[ptr] =1;
                        }
                        // active list is full
                        else if(frame_in_active == active_num)
                        {
                            // if active_tail ref=1 , move to active head
                            while(active_tail->reference)
                            {
                                remove = active_tail;
                                active_tail = active_tail->pre;
                                active_tail->next = NULL;

                                remove->reference = 0;
                                remove->next = active_head;
                                active_head->pre = remove;
                                active_head = remove;
                                active_head->pre = NULL;
                            }

                            // if active_tail ref = 0 , move to inactive
                            remove = active_tail;
                            active_tail = active_tail->pre;
                            active_tail->next = NULL;

                            remove->next = inactive_head;
                            inactive_head->pre = remove;
                            remove->pre = NULL;
                            inactive_head = remove;

                            //frame[remove->pfi][1]=0;
                            list_type[remove->pfi] =0;
                            frame_in_active--;
                            frame_in_inactive++;

                            // put temp in active list head
                            temp->reference = 0;
                            temp->next = active_head;
                            active_head->pre = temp;
                            temp->pre = NULL;
                            active_head = temp;
                            list_type[ptr] = 1;
                        }
                        else
                        {
                            // put temp in active list head
                            temp->reference = 0;
                            temp->next = active_head;
                            active_head->pre = temp;
                            temp->pre = NULL;
                            active_head = temp;
                            list_type[ptr] = 1;
                        }

                        frame_in_active++;
                        frame_in_inactive--;
                    }
                    // reference = 0
                    // set ref bit and move the page to inactive list head
                    else
                    {
                        if(temp->next != NULL && temp->pre != NULL)
                        {
                            temp->pre->next = temp->next;
                            temp->next->pre = temp->pre;

                            temp->reference = 1;
                            temp->next = inactive_head;
                            temp->pre = NULL;

                            inactive_head->pre = temp;
                            inactive_head = temp;
                        }
                        else if(temp->next != NULL)
                        {
                            temp->reference = 1;
                        }
                        else if(temp->pre != NULL)
                        {
                            temp->pre->next = NULL;
                            inactive_tail = inactive_tail->pre;

                            temp->reference = 1;
                            temp->next = inactive_head;
                            temp->pre = NULL;

                            inactive_head->pre = temp;
                            inactive_head = temp;
                        }
                    }
                }
            }
            // not in list => miss
            else
            {
                M++;
                // inactive list is empty
                if(frame_in_inactive==0)
                {
                    space=0;
                    temp = inactive_head;
                    temp->next = NULL;
                    temp->pre = NULL;
                    temp->pfi = space;
                    temp->reference = 1;

                    inactive_tail = temp;
                    PFI[space] = target;
                    list_type[space] = 0;
                    frame_in_inactive++;
                }
                // inactive list is full
                else if(frame_in_inactive == inactive_num)
                {
                    temp = (struct node*)malloc(sizeof(struct node*));
                    temp = inactive_tail;
                    // if ref = 1 , move to inactive head
                    while(temp->reference ==1)
                    {
                        temp->reference = 0;
                        inactive_tail = inactive_tail->pre;
                        inactive_tail->next = NULL;

                        temp->next = inactive_head;
                        inactive_head->pre = temp;
                        inactive_head = temp;
                        inactive_head->pre = NULL;

                        temp = inactive_tail;
                    }

                    evicted = PFI[temp->pfi];
                    PFI[temp->pfi] = -1;
                    list_type[temp->pfi]= 0;
                    space = temp->pfi;

                    // delete from inactive list =? swap out
                    if(temp->next!=NULL && temp->pre!=NULL)
                    {
                        temp->pre->next = temp->next;
                        temp->next->pre = temp->pre;
                    }
                    else if(temp->next!=NULL)
                    {
                        temp->next->pre = NULL;
                        inactive_head = inactive_head->next;
                    }
                    else if(temp->pre!=NULL)
                    {
                        temp->pre->next = NULL;
                        inactive_tail = inactive_tail->pre;
                    }

                    frame_in_inactive--;

                    for(int k=0; k<physical_frame; k++)
                    {
                        if(PFI[k]==-1)
                        {
                            space = k;
                            break;
                        }
                    }

                    temp = (struct node*)malloc(sizeof(struct node*));
                    temp->pre = NULL;
                    temp->next = inactive_head;
                    inactive_head->pre = temp;

                    temp->reference  = 1;
                    temp->pfi = space;
                    inactive_head = temp;

                    PFI[space] = target;
                    list_type[space] = 0;
                    frame_in_inactive++;
                }
                // inactive list still can put new node
                else
                {
                    for(int k=0; k<physical_frame; k++)
                    {
                        if(PFI[k]==-1)
                        {
                            space = k;
                            break;
                        }
                    }

                    temp = (struct node*)malloc(sizeof(struct node*));
                    temp->pre = NULL;
                    temp->next = inactive_head;

                    inactive_head->pre = temp;
                    temp->reference = 1;
                    temp->pfi = space;
                    inactive_head = temp;
                    PFI[space] = target;
                    list_type[space] = 0;
                    frame_in_inactive++;
                }

                // disk process
                if(evicted != -1)
                {
                    for(int g=0; g<virtual_page; g++)
                    {
                        if(disk[g]==-1)
                        {
                            disk[g] = evicted;
                            Destination = g;
                            break;
                        }
                    }
                }

                for(int l=0; l<virtual_page; l++)
                {
                    if(disk[l]==target)
                    {
                        disk[l] = -1;
                        Source = l;
                        break;
                    }
                }

                printf("Miss, %d, %d>>%d, %d<<%d\n",space,evicted,Destination,target,Source);
            }
        }
        miss_rate = M;
        miss_rate /= (M+H);
        printf("Page Fault Rate: %.3f\n",miss_rate);
    }
}