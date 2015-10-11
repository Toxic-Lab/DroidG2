package org.toxiclab.droidg2.jni;

public class toxiclab_downloadResult {
	public int id;
	public String name;
	public String path;
	public String size;
	public String hrsize;
	public String progress;
	public String downloaders;
	public String rate;
	public String sha1;
	public String ttr;
	public String md5;
	public String ed2k;
	public String bp;
	public String tiger_min;
	
	public toxiclab_fileRange[] range;
	public toxiclab_sourceHost[] hosts;
	
	public toxiclab_downloadResult(
			int id, String name, String path, String size, String hrsize, String progress,
			String downloaders, String rate, String sha1, String ttr, String md5, String ed2k,
			String bp, String tiger_min, toxiclab_fileRange[] range, toxiclab_sourceHost[] host){
		this.id = id;
		this.name = name;
		this.path = path;
		this.size = size;
		this.hrsize = hrsize;
		this.progress = progress;
		this.downloaders = downloaders;
		this.rate = rate;
		this.sha1 = sha1;
		this.ttr = ttr;
		this.md5 = md5;
		this.ed2k = ed2k;
		this.tiger_min = tiger_min;
	}
}
