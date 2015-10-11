package org.toxiclab.droidg2;

import java.net.URLEncoder;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

public class SearchActivity extends Activity{
	private SA_Runnable r;
	private Thread t;
	private Button btnAdd;
	private EditText txtSearch;
	private ListView lv;
	private SA_Adapter adapter;
	private int context_ID = 0;
	public static String saResult;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.search);
        
        btnAdd = (Button)findViewById(R.id.Button01);
        txtSearch = (EditText)findViewById(R.id.EditText01);
        lv = (ListView)findViewById(R.id.ListView01);
        registerForContextMenu(lv);
        saResult = getString(R.string.sitemresult);
        
        btnAdd.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				try{
					String string = txtSearch.getText().toString();
					if(string.trim().length() == 0)
						return;
					txtSearch.setText("");
					main._self.createSearch(string, "", "", "", "");
				}
				catch(Exception ex){
					ex.printStackTrace();
				}
			}
        	
        });
        
        adapter = new SA_Adapter(this, R.layout.sever_item);
        lv.setAdapter(adapter);
        lv.setOnItemClickListener(new OnItemClickListener(){

			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				// TODO Auto-generated method stub
				SearchEntry s = adapter.getItem((int)arg3);
				String sl = String.valueOf(s.id);
				Intent intent = new Intent();
				intent.setClass(SearchActivity.this, Search_2_Activity.class);
				intent.putExtra("ID", sl);
				intent.putExtra("Result", s.result);
				startActivity(intent);
			}
        	
        });
        r = new SA_Runnable();
    }
    
    public void onPause(){
    	super.onPause();
    	if(r != null)
    		//main.m_handler.removeCallbacks(r);
    		r.cancel();
    }
    
    public void onResume(){
    	super.onResume();
    	r = new SA_Runnable();
		main.m_timer.scheduleAtFixedRate(r, 1, 1000);
    }
    
    public void onStop(){
    	super.onStop();
    	if(r != null)
    		//main.m_handler.removeCallbacks(r);
    		r.cancel();
    }
    
    class SA_Runnable extends TimerTask{

    	@Override
    	public void run() {
    		final Search h = MyHTMLParser.parseSearch();
    		
    		//do the update here
    		main.m_handler.post(new Runnable(){
    			public void run(){
    				adapter.updateFromSearch(h);
    			}
    		});
    	}
    	
    }    
    
    public static String createSearchQuery(String name, String min, String max, String media, String filetype){
    	try{
	    	StringBuffer sb = new StringBuffer();
	    	if(name != null){
	    		sb.append(URLEncoder.encode("dn", "UTF-8"));
	    		sb.append("=");
	    		sb.append(URLEncoder.encode(name, "UTF-8"));
	    	}
	    	if(min != null){
	    		if(sb.length() != 0) sb.append("&");
	    		sb.append(URLEncoder.encode("min", "UTF-8"));
	    		sb.append("=");
	    		sb.append(URLEncoder.encode(min, "UTF-8"));
	    	}
	    	if(max != null){
	    		if(sb.length() != 0) sb.append("&");
	    		sb.append(URLEncoder.encode("max", "UTF-8"));
	    		sb.append("=");
	    		sb.append(URLEncoder.encode(max, "UTF-8"));
	    	}
	    	if(media != null){
	    		if(sb.length() != 0) sb.append("&");
	    		sb.append(URLEncoder.encode("media", "UTF-8"));
	    		sb.append("=");
	    		sb.append(URLEncoder.encode(media, "UTF-8"));
	    	}
	    	if(filetype != null){
	    		if(sb.length() != 0) sb.append("&");
	    		sb.append(URLEncoder.encode("ext", "UTF-8"));
	    		sb.append("=");
	    		sb.append(URLEncoder.encode(filetype, "UTF-8"));
	    	}
	    	return sb.toString();
    	}
    	catch(Exception ex){
    		ex.printStackTrace();
    		return null;
    	}
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu){
    	MenuInflater inflater = getMenuInflater();
    	inflater.inflate(R.menu.search_menu, menu);
    	return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item){
    	switch(item.getItemId()){
    	case R.id.item1:
			Intent intent = new Intent();
			intent.setClass(this, AdvanceSearch.class);
			startActivity(intent);
    		return true;
    	case R.id.item2:
    		finish();
    		return true;
    	default:
    		return super.onContextItemSelected(item);
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
	    SearchEntry se = adapter.getItem(info1.position);
	    context_ID = se.id;
	    MenuItem resume = menu.findItem(R.id.item2);
	    MenuItem stop = menu.findItem(R.id.item3);
	    if(se.rate <= 0){
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
		  main._self.deleteSearch(context_ID);
		  return true;
	  case R.id.item2:
		  main._self.resumeSearch(context_ID);
		  return true;
	  case R.id.item3:
		  main._self.pauseSearch(context_ID);
		  return true;
	  default:
	    return super.onContextItemSelected(item);
	  }
	}
	
    
}
