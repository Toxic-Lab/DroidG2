package org.toxiclab.droidg2.jni;

public class toxiclab_sourceHost {
	public String source_number;
	public String source_endpoint;
	public String source_connection;
	public String source_max;
	public String source_after;
	public String source_nick;
	
	public toxiclab_sourceHost(
			String source_number, String source_endpoint, String source_connection,
			String source_max , String source_after, String source_nick){
		this.source_after = source_after;
		this.source_connection = source_connection;
		this.source_endpoint = source_endpoint;
		this.source_max = source_max;
		this.source_nick = source_nick;
		this.source_number = source_number;
	}
}
