package org.toxiclab.droidg2.jni;

public class toxiclab_searchResult{
	public String name;
	public int rid;
	public int sid;
	public String size;
	public String type;
	public int sources;
	
	public toxiclab_searchResult(
			String name, int rid, int sid, String size, String type, int sources){
		this.name = name;
		this.rid = rid;
		this.sid = sid;
		this.size = size;
		this.type = type;
		this.sources = sources;
	}
}

