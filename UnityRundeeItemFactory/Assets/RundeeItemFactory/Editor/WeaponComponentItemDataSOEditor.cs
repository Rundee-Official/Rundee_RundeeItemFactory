using UnityEditor;
using UnityEditorInternal;
using UnityEngine;

[CustomEditor(typeof(WeaponComponentItemDataSO))]
public class WeaponComponentItemDataSOEditor : Editor
{
    private SerializedProperty componentTypeProp;
    private SerializedProperty magazineCapacityProp;
    private SerializedProperty loadedRoundsProp;

    private ReorderableList loadedRoundsList;
    private bool showLoadPlan = true;

    private void OnEnable()
    {
        componentTypeProp = serializedObject.FindProperty("componentType");
        magazineCapacityProp = serializedObject.FindProperty("magazineCapacity");
        loadedRoundsProp = serializedObject.FindProperty("loadedRounds");

        if (loadedRoundsProp != null)
        {
            loadedRoundsList = new ReorderableList(serializedObject, loadedRoundsProp, true, true, true, true);
            loadedRoundsList.drawHeaderCallback = rect =>
            {
                EditorGUI.LabelField(rect, "Loaded Rounds (Top -> Bottom)");
            };

            loadedRoundsList.drawElementCallback = (rect, index, active, focused) =>
            {
                SerializedProperty element = loadedRoundsProp.GetArrayElementAtIndex(index);
                rect.y += 2;
                float lineHeight = EditorGUIUtility.singleLineHeight;
                float labelWidth = 85f;
                float spacing = 4f;

                Rect orderRect = new Rect(rect.x, rect.y, labelWidth, lineHeight);
                EditorGUI.PropertyField(orderRect, element.FindPropertyRelative("orderIndex"), new GUIContent("Order"));

                Rect countRect = new Rect(orderRect.xMax + spacing, rect.y, labelWidth, lineHeight);
                EditorGUI.PropertyField(countRect, element.FindPropertyRelative("roundCount"), new GUIContent("Count"));

                Rect ammoIdRect = new Rect(countRect.xMax + spacing, rect.y, rect.width - (labelWidth * 2f + spacing * 2f), lineHeight);
                EditorGUI.PropertyField(ammoIdRect, element.FindPropertyRelative("ammoId"), GUIContent.none);

                Rect displayRect = new Rect(rect.x, rect.y + lineHeight + spacing, rect.width, lineHeight);
                EditorGUI.PropertyField(displayRect, element.FindPropertyRelative("ammoDisplayName"), new GUIContent("Display Name"));

                Rect notesRect = new Rect(rect.x, displayRect.y + lineHeight + spacing, rect.width, lineHeight);
                EditorGUI.PropertyField(notesRect, element.FindPropertyRelative("ammoNotes"), new GUIContent("Notes"));
            };

            loadedRoundsList.elementHeightCallback = index =>
            {
                float baseHeight = EditorGUIUtility.singleLineHeight;
                return baseHeight * 3f + 12f;
            };

            loadedRoundsList.onAddCallback = list =>
            {
                int index = loadedRoundsProp.arraySize;
                loadedRoundsProp.InsertArrayElementAtIndex(index);
                SerializedProperty element = loadedRoundsProp.GetArrayElementAtIndex(index);
                element.FindPropertyRelative("orderIndex").intValue = index;
                element.FindPropertyRelative("roundCount").intValue = 1;
                element.FindPropertyRelative("ammoId").stringValue = string.Empty;
                element.FindPropertyRelative("ammoDisplayName").stringValue = string.Empty;
                element.FindPropertyRelative("ammoNotes").stringValue = string.Empty;
            };
        }
    }

    public override void OnInspectorGUI()
    {
        serializedObject.Update();

        EditorGUILayout.PropertyField(serializedObject.FindProperty("id"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("displayName"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("category"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("rarity"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("maxStack"));

        EditorGUILayout.PropertyField(componentTypeProp);
        EditorGUILayout.PropertyField(serializedObject.FindProperty("compatibleSlots"), true);

        bool isMagazine = string.Equals(componentTypeProp.stringValue, "Magazine", System.StringComparison.OrdinalIgnoreCase);

        using (new EditorGUI.DisabledScope(!isMagazine))
        {
            EditorGUILayout.PropertyField(magazineCapacityProp);
            EditorGUILayout.PropertyField(serializedObject.FindProperty("caliber"));
            EditorGUILayout.PropertyField(serializedObject.FindProperty("magazineType"));
        }

        EditorGUILayout.PropertyField(serializedObject.FindProperty("subSlots"), true);

        EditorGUILayout.PropertyField(serializedObject.FindProperty("damageModifier"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("recoilModifier"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("ergonomicsModifier"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("accuracyModifier"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("weightModifier"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("muzzleVelocityModifier"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("effectiveRangeModifier"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("penetrationModifier"));

        EditorGUILayout.PropertyField(serializedObject.FindProperty("hasBuiltInRail"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("railType"));

        EditorGUILayout.PropertyField(serializedObject.FindProperty("description"));

        EditorGUILayout.Space();

        if (!isMagazine)
        {
            EditorGUILayout.HelpBox("Loaded rounds apply only to Magazine components.", MessageType.Info);
        }
        else if (loadedRoundsList != null)
        {
            showLoadPlan = EditorGUILayout.Foldout(showLoadPlan, "Magazine Load Plan", true);
            if (showLoadPlan)
            {
                EditorGUILayout.HelpBox("Order index 0 fires first. Sum of counts should not exceed capacity. Use ammo IDs from the Ammo dataset (e.g., Ammo_556_AP).", MessageType.None);
                loadedRoundsList.DoLayoutList();

                EditorGUILayout.BeginHorizontal();
                if (GUILayout.Button("Normalize Order"))
                {
                    NormalizeOrderIndices();
                }
                if (GUILayout.Button("Clear Load Plan"))
                {
                    loadedRoundsProp.ClearArray();
                }
                EditorGUILayout.EndHorizontal();
            }
        }

        serializedObject.ApplyModifiedProperties();
    }

    private void NormalizeOrderIndices()
    {
        for (int i = 0; i < loadedRoundsProp.arraySize; i++)
        {
            SerializedProperty element = loadedRoundsProp.GetArrayElementAtIndex(i);
            element.FindPropertyRelative("orderIndex").intValue = i;
        }
    }
}












