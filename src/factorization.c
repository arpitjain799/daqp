#include "factorization.h"
void update_LDL_add(Workspace *work){
  work->sing_ind = EMPTY_IND;
  int i,j,disp,disp2;
  int add_offset = (NX)*(work->add_ind);
  int new_L_start= ARSUM(work->n_active);
  c_float sum;

  // di <-- Mi' Mi
  sum = 0;
  disp = add_offset;
  // If normalized this will always be 1...
  for(i=0;i<NX;i++,disp++)
	sum+=(work->M[disp])*(work->M[disp]);
  work->D[work->n_active] = sum;
  
  if(work->n_active==0)
	return;

  // store l <-- Mk* m
  for(i=0;i<work->n_active;i++){
	disp = add_offset;
	disp2 = NX*work->WS[i];
	sum=0;
	for(j=0;j<NX;j++,disp++,disp2++)
	  sum +=work->M[disp2]*work->M[disp];
	work->L[new_L_start+i] = sum;
  }
  //Forward substitution: l <-- L\(Mk*m)  
  for(i=0,disp=0; i<work->n_active; i++){
	sum = work->L[new_L_start+i];
	for(j=0; j<i; j++)
	  sum -= work->L[disp++]*work->L[new_L_start+j]; 
	work->L[new_L_start+i] = sum;
	disp++; //Skip diagonal elements (which is 1)
  }
  
  // Scale: l_i <-- l_i/d_i
  // Update d_new -= l'Dl
  sum = work->D[work->n_active];
  for (i =0,disp=new_L_start; i<work->n_active;i++,disp++){
	work->L[disp] /= work->D[i];  
	sum -= (work->L[disp]*work->D[i])*work->L[disp];
  }
  work->D[work->n_active]=sum;

  // Check for singularity
  if(work->D[work->n_active]<work->settings->zero_tol||work->n_active==work->n){
	work->sing_ind=work->n_active;
	work->D[work->n_active]=0;
  }
}

void update_LDL_remove(Workspace *work){
  if(work->n_active==work->rm_ind+1)
	return;
  int i, j, r, old_disp, new_disp, w_count, n_update=work->n_active-work->rm_ind-1;
  c_float* w = &work->zldl[work->rm_ind]; // Since zldl will be obsolete use that memory to save some allocations..
  // Extract parts to keep/update in L & D
  new_disp=ARSUM(work->rm_ind);
  old_disp=new_disp+(work->rm_ind+1);
  w_count= 0;
  // Remove column rm_ind (and add parts of L in its new place)
  // I.e., copy row i into i-1
  for(i = work->rm_ind+1;i<work->n_active;old_disp++,new_disp++,i++) //(disp++ skips blank element)..
	for(j=0;j<i;j++){ 
	  if(j!=work->rm_ind)
		work->L[new_disp++]=work->L[old_disp++];
	  else
		w[w_count++] = work->L[old_disp++];
	}
  // Algorithm C1 in Gill 1974 for low-rank update of LDL 
  // (L2 block...)
  // TODO the disp can most likely be done cleaner...
  c_float p,beta,d_bar,alpha=work->D[work->rm_ind];
  // i - Element/row to update|j - Column which is looped over|r - Row to loop over
  old_disp=ARSUM(work->rm_ind)+work->rm_ind;
  for(j = 0, i=work->rm_ind; j<n_update;j++,i++){
	p=w[j];
	d_bar = work->D[i+1]+alpha*p*p; 
	beta = p*alpha/d_bar;
	alpha =work->D[i+1]*alpha/d_bar;
	work->D[i] = d_bar;
	// This means that singularity was not detected correctly before (numerical erros)
	// TODO Do some kind of "repair" step. 
	if(d_bar<work->settings->zero_tol){
	  //work->D[i]=0;
	  work->sing_ind=i;
	}
	old_disp+=i+1; 
	for(r=j+1, new_disp=old_disp+j;r<n_update;r++){
	  w[r] -= p*work->L[new_disp];//instead, initialize new_disp+j
	  work->L[new_disp]+= beta*w[r]; //Use sum to block register
	  new_disp+=work->rm_ind+r+1; //Update to the id which starts the next row in L
	}
  }
}
