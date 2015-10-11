package org.toxiclab.droidg2;

import java.io.DataInputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;

import org.toxiclab.droidg2.jni.toxiclab_download;
import org.toxiclab.droidg2.jni.toxiclab_hub;
import org.toxiclab.droidg2.jni.toxiclab_incoming;
import org.toxiclab.droidg2.jni.toxiclab_search;
import org.toxiclab.droidg2.jni.toxiclab_searchResult;

public class MyHTMLParser {
	public static String sortby_source = "sources";
	public static String sortby_name = "name";
	public static String sortby_size = "size";
	public static String sortby_done = "done";
	public static String sortby_rate = "rate";
	public static String sortby_priority = "priority";
	public static String sortby_none = "";	
	
    public static int FILETYPE_AUDIO = 1;
    public static int FILETYPE_IMAGE = 2;
    public static int FILETYPE_ZIP = 3;
    public static int FILETYPE_DOC = 4;
    public static int FILETYPE_VIDEO = 5;
    
    public static int getType(String type){
    	if(type.compareTo("video") == 0)
    		return FILETYPE_VIDEO;
    	else if(type.compareTo("audio") == 0)
    		return FILETYPE_AUDIO;
    	else if(type.compareTo("archive") == 0 || type.compareTo("msapp") == 0|| type.compareTo("cdimage") == 0)
    		return FILETYPE_ZIP;
    	else if(type.compareTo("document") == 0 )
    		return FILETYPE_DOC;
    	else if(type.compareTo("picture") == 0  )
    		return FILETYPE_IMAGE;
    	else
    		return FILETYPE_DOC;
	}
	
	public static Home parseHome( ){
		toxiclab_hub[] connected = main._self.showHubs();
		toxiclab_hub[] connecting = main._self.showHandshake();
		Home home = new Home();
		
		for(int i=0; i<connected.length; ++i){
			home.server.add(connected[i].address);
			home.status.add(connected[i].status);
		}
		
		for(int i=0; i<connecting.length; ++i){
			home.server.add(connecting[i].address);
			home.status.add(connecting[i].status);
		}
		
		return home;
	}
	
	public static Search parseSearch( ){
		try{
			Search search = new Search();
			
			toxiclab_search[] temp = main._self.showSearch();
			for(int i=0; i<temp.length; ++i){
				search.result.add( temp[i].hits);
				search.title.add(temp[i].criteria);
				search.ID.add(temp[i].id);
				search.rate.add(Integer.parseInt(temp[i].active));
			}
			
			return search;
		}
		catch(Exception ex){
			ex.printStackTrace();
			return null;
		}
	}
	
	public static Incoming parseIncoming(){
		Incoming in = new Incoming();
		try{
			toxiclab_incoming[] temp = main._self.showIncoming();
			for(int i=0; i<temp.length; ++i){
				in.name.add(temp[i].name);
				in.size.add(temp[i].size);
				in.type.add(temp[i].type);
				in.parent.add(temp[i].parent);
			}
			return in;
		}
		catch(Exception ex){
			ex.printStackTrace();
			return null;
		}
	}
	
	public static Search_2 parseSearch_2(int id, String sortBy, boolean forward){
		String b = null;
		try{
			Search_2 search = new Search_2();
			toxiclab_searchResult[] r = main._self.showSearchResult(id, sortBy, forward);
			for(int i=0; i<r.length; ++i){
				search.link.add(null);
				search.size.add(r[i].size);
				search.source.add(String.valueOf(r[i].sources));
				search.title.add(r[i].name);
				search.type.add(r[i].type);
				search.sid.add(r[i].sid);
				search.rid.add(r[i].rid);
			}
			return search;
		}
		catch(Exception ex){
			System.err.println(b);
			ex.printStackTrace();
			return null;
		}
		
	}
	
	public static Download parseDownload(){
		try{
			Download down = new Download();
			
			toxiclab_download[] temp = main._self.showAllDownload(sortby_name, false);
			
			for(int i=0; i<temp.length; ++i){
				down.name.add(temp[i].download_name);
				down.done.add(temp[i].downloaded_bytes);
				down.rate.add(temp[i].download_rate);
				down.size.add(temp[i].download_size);
				down.status.add(temp[i].status);
				down.ID.add(temp[i].download_id);
				down.source.add(String.format("%d/%d", temp[i].active_sources, temp[i].sources));
			}
			
			return down;
		}
		catch(Exception ex){
			ex.printStackTrace();
			return null;
		}
	}
	
	public static Download_2 parseDownload_2(String a){
		Download_2 down = null;

		return down;
	}
	
    public static String getWebPage(String uri){
    	try{
	    	URL url;
	    	URLConnection urlConn;
	    	DataInputStream dis;
	    	
	    	url = new URL(uri);
	    	urlConn = url.openConnection();
	    	urlConn.setDoInput(true);
	    	urlConn.setUseCaches(false);
	    	dis = new DataInputStream(urlConn.getInputStream());
	    	String s;
	    	
	    	StringBuffer sb = new StringBuffer();
	    	while((s = dis.readLine()) != null){
	    		sb.append(s);
	    	}
	    	dis.close();
	    	return sb.toString();
    	}
    	catch(Exception ex){
    		ex.printStackTrace();
    		return null;
    	}
    }
    
}

class Home{
	ArrayList<String> server = new ArrayList<String>();
	ArrayList<String> status = new ArrayList<String>();
}

class Search{
	ArrayList<String> title = new ArrayList<String>();
	ArrayList<String> result = new ArrayList<String>();
	ArrayList<Integer> ID = new ArrayList<Integer>();
	ArrayList<Integer> rate = new ArrayList<Integer>();
}

class Download{
	ArrayList<String> name = new ArrayList<String>();
	ArrayList<String> size = new ArrayList<String>();
	ArrayList<String> done = new ArrayList<String>();
	ArrayList<String> rate = new ArrayList<String>();
	ArrayList<String> source = new ArrayList<String>();
	ArrayList<String> status = new ArrayList<String>();
	ArrayList<Integer> ID = new ArrayList<Integer>();
}

class Search_2{
	ArrayList<String> type = new ArrayList<String>();
	ArrayList<URL> link = new ArrayList<URL>();
	ArrayList<String> title = new ArrayList<String>();
	ArrayList<String> size = new ArrayList<String>();
	ArrayList<String> source = new ArrayList<String>();
	ArrayList<Integer> sid = new ArrayList<Integer>();
	ArrayList<Integer> rid = new ArrayList<Integer>();
}

class Download_2{
	
}

class Incoming{
	ArrayList<String> name = new ArrayList<String>();
	ArrayList<String> size = new ArrayList<String>();
	ArrayList<String> type = new ArrayList<String>();
	ArrayList<String> parent = new ArrayList<String>();
}
