package org.toxiclab.droidg2;

import java.util.TimerTask;

import android.app.Activity;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;

public class DownloadActivity extends Activity {
	private DA_Runnable r;
	private Thread t;
	private ListView lv;
	private DA_Adapter adapter;
	private int context_ID;
	
	public static String[] sarray;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.download);
        
        Resources res = getResources();
        sarray = res.getStringArray(R.array.downstatus);
        
        lv = (ListView)findViewById(R.id.ListView01);
        adapter = new DA_Adapter(this, R.layout.download);
        lv.setAdapter(adapter);
        registerForContextMenu(lv);
        r = new DA_Runnable();
    }

	public void onPause(){
    	super.onPause();
    	if(r != null)
    		r.cancel();
    }
    
    public void onResume(){
    	super.onResume();
    	r = new DA_Runnable();
    	main.m_timer.scheduleAtFixedRate(r, 1, 1000);
    }
    
    public void onStop(){
    	super.onStop();
    	if(r != null)
    		r.cancel();
    }
    
    class DA_Runnable extends TimerTask{

    	@Override
    	public void run() {
    		final Download h = MyHTMLParser.parseDownload();
    		
    		//do the update here
    		main.m_handler.post(new Runnable(){
    			public void run(){
    				adapter.updateFromDownload(h);
    			}
    		});
    	}

    } 
        
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v,
	    ContextMenuInfo menuInfo) {
	  if (v.getId()==R.id.ListView01) {
	    AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo)menuInfo;
	    
	    menu.setHeaderTitle("Actions");
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.search_content_menu, menu);
	    AdapterView.AdapterContextMenuInfo info1 = (AdapterView.AdapterContextMenuInfo)menuInfo;
	    DownloadEntry se = adapter.getItem(info1.position);
	    context_ID = se.id;
	    MenuItem resume = menu.findItem(R.id.item2);
	    MenuItem stop = menu.findItem(R.id.item3);
	    if(se.status.compareTo("paused") == 0){
	    	resume.setEnabled(true);
	    	stop.setEnabled(false);
	    }
	    else{
	    	resume.setEnabled(false);
	    	stop.setEnabled(true);
	    }
	  }
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
	  AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
	  switch (item.getItemId()) {
	  case R.id.item1:
		  main._self.deleteDownload(context_ID);
		  return true;
	  case R.id.item2:
		  main._self.resumeDownload(context_ID);
		  return true;
	  case R.id.item3:
		  main._self.pauseDownload(context_ID);
		  return true;
	  default:
	    return super.onContextItemSelected(item);
	  }
	}
	
}
