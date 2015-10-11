package org.toxiclab.droidg2.jni;

public class toxiclab_download {
	public String download_name;
	public String download_size;
	public String download_rate;
	public int sources;
	public int active_sources;
	public String progress;
	public int download_id;
	public int priority;
	public boolean dl_paused;
	public String status;
	public String downloaded_bytes;
	public toxiclab_download(String dn, String ds, String dr, int ss, int as, String pg, int did, int pr, boolean dlp, String sts, String downloaded_bytes){
		download_name = dn;
		download_size = ds;
		download_rate = dr;
		sources = ss;
		active_sources = as;
		progress = pg;
		download_id = did;
		priority = pr;
		dl_paused = dlp;
		status = sts;
		this.downloaded_bytes = downloaded_bytes;
	}
}
