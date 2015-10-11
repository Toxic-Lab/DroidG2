package org.toxiclab.droidg2.jni;

public class toxiclab_search{
	public String criteria;
	public String hits;
	public String active;
	public String endless;
	public String extensions;
	public int id;
	
	public toxiclab_search(String criteria, String hits, String active, String endless, String extensions, int id){
		this.criteria = criteria;
		this.hits = hits;
		this.active = active;
		this.endless = endless;
		this.extensions = extensions;
		this.id = id;
	}
}
