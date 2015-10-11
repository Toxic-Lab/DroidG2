package org.toxiclab.droidg2;

import java.net.URL;
import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class DA_Adapter extends ArrayAdapter<DownloadEntry>{
	private Context m_context;
	private int m_id;
	private static DownloadEntry[] ss = null;
	private static DA_Adapter _self = null;
	
	public DA_Adapter(Context context, int textViewResourceId) {
		super(context, textViewResourceId);
		// TODO Auto-generated constructor stub
		m_context = context;
		m_id = textViewResourceId;
		_self = this;
	}
	
	@Override
	public int getCount(){
		if(ss == null) return 0;
		else return ss.length;
	}
	
	@Override
	public DownloadEntry getItem(int a){
		return ss[a];
	}
	
	@Override
	public View getView(int a, View convertView, ViewGroup parent){
		View result = null;
		DownloadEntry _s = null;
		if(convertView == null){
			//inflate the xml view
			LayoutInflater inflate = (LayoutInflater)
            		m_context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			RelativeLayout layout = (RelativeLayout)inflate.inflate(R.layout.download_item, null);
			result = layout;
		}
		else{
			//use the view directly
			result = convertView;
		}
		
		_s = getItem(a);
		ImageView type = (ImageView)result.findViewById(R.id.ImageView01);
		TextView t1 = (TextView)result.findViewById(R.id.TextView01);
		TextView t2 = (TextView)result.findViewById(R.id.TextView02);
		TextView t3 = (TextView)result.findViewById(R.id.TextView03);
		TextView t4 = (TextView)result.findViewById(R.id.TextView04);
		TextView t5 = (TextView)result.findViewById(R.id.textView1);
		
		int typex = getType(_s.name);
		if(typex == FILETYPE_AUDIO)
			type.setImageResource(R.drawable.music);
		else if(typex == FILETYPE_IMAGE)
			type.setImageResource(R.drawable.picture);
		else if(typex == FILETYPE_ZIP)
			type.setImageResource(R.drawable.pack);
		else if(typex == FILETYPE_VIDEO)
			type.setImageResource(R.drawable.video);
		else
			type.setImageResource(R.drawable.document);

		t1.setText(_s.name);
		t2.setText(String.format("%s/%s", _s.done, _s.size));
		t3.setText(_s.rate);
		
		t4.setText(getStatus(_s.status));
		t5.setText(_s.source);		
		
		return result;
	}
	
	public static String getStatus(String input){
		if(input.compareTo("transfer") == 0)
			return DownloadActivity.sarray[0];
		else if(input.compareTo("hash") == 0)
			return DownloadActivity.sarray[1];
		else if(input.compareTo("move") == 0)
			return DownloadActivity.sarray[2];
		else if(input.compareTo("paused") == 0)
			return DownloadActivity.sarray[3];
		else if(input.compareTo("queued") == 0)
			return DownloadActivity.sarray[4];
		else if(input.compareTo("error") == 0)
			return DownloadActivity.sarray[5];
		return "";		
	}
	
	public static void updateFromDownload(Download h){
		if(h == null) return;
		ArrayList<DownloadEntry> als = new ArrayList<DownloadEntry>();
		for(int i=0; i<h.name.size(); ++i){
			als.add(new DownloadEntry(h.ID.get(i),
					 h.name.get(i), h.size.get(i), h.done.get(i), h.rate.get(i), h.source.get(i), h.status.get(i)));
		}
		ss = als.toArray(new DownloadEntry[0]);
		if(_self != null)
			_self.notifyDataSetChanged();
	}
	
    public static int FILETYPE_AUDIO = 1;
    public static int FILETYPE_IMAGE = 2;
    public static int FILETYPE_ZIP = 3;
    public static int FILETYPE_DOC = 4;
    public static int FILETYPE_VIDEO = 5;
    
    public static int getType(String fn){
    	boolean right = false;
    	
    	String[] type = AUDIO_FILE_EXT;
    	for(int i=0; i<type.length; ++i)
    		if(fn.endsWith(type[i]))
    			return FILETYPE_AUDIO;
    	type = DOCUMENTS_FILE_EXT;
    	type = IMAGE_FILE_EXT;
    	for(int i=0; i<type.length; ++i)
    		if(fn.endsWith(type[i]))
    			return FILETYPE_IMAGE;
    	type = PROGRAM_FILE_EXT;
    	for(int i=0; i<type.length; ++i)
    		if(fn.endsWith(type[i]))
    			return FILETYPE_ZIP;
    	type = VIDEO_FILE_EXT;
    	for(int i=0; i<type.length; ++i)
    		if(fn.endsWith(type[i]))
    			return FILETYPE_VIDEO;
    	return FILETYPE_DOC;
    }
    
    public static final String[] AUDIO_FILE_EXT = new String[] { 
        "aif", "aifc", "aiff", "ape", "apl", "au", "iso", "lqt",
        "mac", "med", "mid", "midi", "mod", "mp3", "mpa", "mpga", "mp1",
        "ogg", "ra", "ram", "rm", "rmi", "rmj", "snd", "vqf", 
        "wav", "wma" };
    public static final String[] VIDEO_FILE_EXT = new String[] { 
        "asf", "avi", "dcr", "div", "divx", "dv", "dvd", "ogm",
        "dvx", "flc", "fli", "flv", "flx", "jve", "m2p", "m2v", "m1v", "mkv", "mng",
        "mov", "mp2", "mp2v", "mp4", "mpe", "mpeg", "mpg", "mpv", "mpv2",
        "nsv", "ogg", "ram", "rm", "rv", "smi", "smil", "swf", "qt", "vcd",
        "vob", "vrml", "wml", "wmv" };
    public static final String[] PROGRAM_FILE_EXT = new String[] { 
        "7z", "ace", "arj", "awk", "bin", "bz2", "cab", "csh",
        "deb", "dmg", "img", "exe", "gz", "gzip", "hqx", "iso", "jar", "lzh", 
        "lha", "mdb", "msi", "msp", "pl", "rar", "rpm", "sh", "shar", "sit",
        "tar", "tgz", "taz", "z", "zip", "zoo", "pl", "ebuild" };
    public static final String[] IMAGE_FILE_EXT = new String[] { 
        "ani", "bmp", "cpt", "cur", "dcx", "dib", "drw",
        "emf", "fax", "gif", "icl", "ico", "iff", "ilbm", "img", "jif",
        "jiff", "jpe", "jpeg", "jpg", "lbm", "mac", "mic", "pbm", "pcd",
        "pct", "pcx", "pic", "png", "pnm", "ppm", "psd", "ras", "rgb", "rle",
        "sgi", "sxd", "svg", "tga", "tif", "tiff", "wmf", "wpg", "xbm", "xcf", 
        "xpm", "xwd" };
    public static final String[] DOCUMENTS_FILE_EXT = new String[] {
        "ans", "asc", "chm", "csv", "dif", "diz", "doc", "eml",
        "eps", "epsf", "hlp", "html", "htm", "info", "latex", "man", "mcw",
        "mht", "mhtml", "odt", "pdf", "ppt", "ps", "rtd", "rtf", "rtt", "sxw", "sxc", 
        "tex", "texi", "txt", "wk1", "wps", "wri", "xhtml", "xls", "xml", "sla", "kwd"
    };
    public static final String[] ROMS_FILE_EXT = new String[] {
        "bin", "smd", "smc", "fig", "srm", "zip", "sav", "rar", "frz", "fra", 
        "zs1", "pcx"
    };
    public static final String[] OPEN_FORMATS_FILE_EXT = new String[] {
        "ogg", "ogm", "tgz", "gz", "tbz", "bz2", "bz", "png", 
        "flac", "tar", "gzip", "txt", "mkv", "odt", "sla", "kwd", "tex"
    };
    public static final String[] META_FILE_EXT = new String[] {
        "magma", "xml", "collection", "torrent", "col"
    };
}

class DownloadEntry{
	public DownloadEntry(int id, String name, String size, String done, String rate, String source, String status){
		this.name = name;
		this.size = size;
		this.done = done;
		this.rate = rate;
		this.source = source;
		this.status = status;
		this.id = id;
	}
	
	public int id;
	public String name;
	public String size;
	public String done;
	public String rate;
	public String source;
	public String status;
}